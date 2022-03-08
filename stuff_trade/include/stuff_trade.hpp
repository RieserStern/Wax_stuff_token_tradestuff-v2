#pragma once

#define CONTRACT_NAME "stuff_trade"

#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/symbol.hpp>
#include <eosio/time.hpp>
#include <vector>
#include <algorithm>

#include <tables/trade/offers.hpp>
#include <tables/trade/seqmgr.hpp>
#include <tables/trade/settings.hpp>
#include <tables/trade/trades.hpp>
#include <tables/arb/disputes.hpp>
#include <tables/stake/stakes.hpp>
#include <tables/stake/totalstake.hpp>
#include <tables/stuff/stuff.hpp>

#include <multilateral.hpp>

using namespace eosio;
using namespace std;

#define TS_SEQTYPE_OFFER "offer"
#define TS_SEQTYPE_TRADE "trade"
#define TS_SEQTYPE_OROUTE "offerroute"
#define TS_SEQTYPE_TROUTE "traderoute"

CONTRACT stuff_trade : public contract
{
  using contract::contract;

  public:
      // Create trade typedef
      class TradeToCreate
      {
        public:
          uint64_t sup_offer_route;
          uint64_t dem_offer_route;
          name supplier;
          uint64_t stuff_id;
          name demandant;
          uint64_t trade_id;

          void setTrade(uint64_t s_route, uint64_t d_route, name supp, uint64_t stuff, name dem, uint64_t trade) { 
            sup_offer_route = s_route; 
            dem_offer_route = d_route; 
            supplier = supp; 
            stuff_id = stuff; 
            demandant = dem; 
            trade_id = trade; 
          } 
      };
      ACTION report(uint64_t id);
      ACTION setconfig(name stuffContract, name stakeContract, name arbContract, name accountsContract);
      ACTION offercreate(uint64_t supply, uint64_t demand, name creator, time_point_sec expires_at);
      ACTION optionaccept(uint64_t supply, uint64_t demand, name creator);
      ACTION getpottrades(uint64_t supply, uint64_t demand, uint64_t routeId, bool trade_contingent);
      ACTION offerrenew(name creator, uint64_t offer_id, time_point_sec new_expires_at);
      ACTION offerdelete(name creator, uint64_t offer_id);
      ACTION offersdelall(vector<uint64_t> stuff_ids);
      ACTION offersdelexp(time_point_sec exp_days);
      ACTION tradecreate(vector<TradeToCreate> trades);
      ACTION markdeliv(uint64_t traderoute_id, string delivery_proof);
      ACTION markreceived(uint64_t traderoute_id);
      ACTION stuffaccept(uint64_t traderoute_id);
      ACTION trdroutedel(vector<uint64_t> traderoute_id);
      ACTION updtdispute(uint64_t route_id, uint64_t dispute_id);

  private:  
    void getpottrades_m(uint64_t supply, uint64_t demand, uint64_t routeId, bool trade_contingent)
    {
      // print("\n==GETPOTTRADES==\n");
      settings_index settings(_self, _self.value);
      auto settings_itr = settings.get();
      name stuffContract = settings_itr.stuffContract;
      name stakeContract = settings_itr.stakeContract;

      // print("Running getpottrades with: ", supply, ", ", demand,", ", routeId, ", ", trade_contingent, "\n");
      
      // Get stuff table via the stuff contract from settings  
      stuff_index stuffTable(stuffContract, stuffContract.value);

      // Get the offers table
      offers_index offerTable(_self, _self.value);

      // See if it's necessary to go through the work of determining potential trade routes
      // We do this by ensuring the supply has at least 1 demand
      // And the demand has at least 1 supply.
      bool checkForTrades = true;

      // Check if supply has another offer where it's the demand
      auto offersByDemand = offerTable.get_index<"bydemand"_n>();
      auto offersByDemandlower = offersByDemand.lower_bound(supply);
      auto offersByDemandupper = offersByDemand.upper_bound(supply);
      if (offersByDemandlower == offersByDemandupper) {
        checkForTrades = false;
      }


      // Check if demand has another offer where it's the supply
      auto offersBySupply = offerTable.get_index<"bysupply"_n>();
      auto offersBySupplyLower = offersBySupply.lower_bound(demand);
      auto offersBySupplyUpper = offersBySupply.upper_bound(demand);
      if (offersBySupplyLower == offersBySupplyUpper) {
        checkForTrades = false;
      }

      // If we shouldnt check for trades, stop here
      if (!checkForTrades) {
          // When trade_contingent is true, return a chain error so the API can be informed
          if (trade_contingent == true)
          {
              check(false, "No trade found.");
          }
          return;
      }

      // Catch the highest stuff id from stuff seqmngr so we know how many vertices our graph needs
      int highest = 1; 
      seqmgr_index stuffseqmngr(stuffContract, stuffContract.value);
      for (auto itr = stuffseqmngr.begin(); itr != stuffseqmngr.end(); itr++)
      {
        if (itr != stuffseqmngr.end() && itr->type == "stuff") {
          highest = itr->id + 1;
        }
      }

      // Create our multilateral graph based on our highest stuff id
      Graph g(highest);

      // Loop through our offer table...
      for (auto itr = offerTable.begin(); itr != offerTable.end(); itr++)
      {
        // ... And add edges to our graph consisting of the supply, demand, and route
        g.addEdge(itr->supply_id, itr->demand_id, itr->route_id);
      }

      // Now get all the paths using this offer's desired demand, supply, and newly-created route
      vector<vector<int>> paths = g.getTradePath(demand, supply, routeId);

      // print("\nTrades found: ", paths.size(), "\n");

      vector<int> selectedPath;
      for(auto it_row =paths.begin(); it_row!=paths.end(); it_row++){
          // Create a list that will store each owner/sum of value
          SumTracker sumTracker;

          // Loop through the offer ids in the current path and 
          // validate the expiration and collateralization of each
          // print ("\nValidating path...\n");
          bool valid = true;
          for(auto it_col=it_row->begin(); it_col!=it_row->end(); it_col++){
              int id = *it_col;
              auto offer = offerTable.find(id);
              auto stuff = stuffTable.find(offer->supply_id);
              if (offer->expires_at  <= current_time_point() || !stuff->collateralized) {
                valid = false;
              }

              // print("[Offer ", id, "]\n");
              // print ("Expires in future: ", offer->expires_at  > current_time_point(), "\n");
              // print ("Collateralized: ", stuff->collateralized, "\n");
              // print ("\n");

              // Add the stuff owner and value to the sum tracker
              // The addSum() method will add the owner/value pair to 
              // the tracker if the owner doesn't already exist in the tracker
              // but if the owner does exist, then it will simply modify the existing record's sum
              Sum s(stuff->account, stuff->value);
              sumTracker.addSum(s);
          }

          // Now that we've stored the stuff owner and value to a list (in the sumTracker),
          // let's loop through the sum tracker and ensure total stake is good for each owner
          name stakeContract = settings_itr.stakeContract;
          total_stake_index totalStakeTable(stakeContract, stakeContract.value);
          for(auto sum=sumTracker.getSums().begin(); sum!=sumTracker.getSums().end(); sum++){
            // print("[SUM]\n");
            // print(sum->getOwner(), "\n");

            auto stake_itr = totalStakeTable.find(sum->getOwner().value);
            // print(sum->getOwner(), " - ", sum->getValue()," - ", stake_itr->available.amount/10000, "\n");

            if (stake_itr != totalStakeTable.end() && (stake_itr->available.amount/10000) < sum->getValue())
            {
              valid = false;
            }
          }
          
          if (valid) {
            selectedPath = *it_row;
            break;
          }
      }

      // If our path is zero, stop here 
      if (selectedPath.size() == 0) {
        // When trade_contingent is true, return a chain error so the API can be informed
        if (trade_contingent == true)
          {
              check(false, "No trade found.");
          }
          return;
      }

      print("---[ TRADE FOUND ]---\n");

      uint64_t tradeid = genseqnum(TS_SEQTYPE_TRADE);

      // Let's create a vector of stuff ids
      vector<uint64_t> stuff_ids;

      // Let's loop through the path (vector of offer route ids)
      vector<TradeToCreate> tradesToCreate;
      
      for(auto offer_id=selectedPath.begin(); offer_id!=selectedPath.end(); offer_id++) {
        int id = *offer_id;

        // Grab the actual offer from the offer table using the route id
        auto offer = offerTable.find(id);

        // Store the stuff_id
        stuff_ids.push_back(offer->demand_id);

        // Let's get the supply route
        // Loop through the selectedPath's again (calling it level 2, hence "l2" prefix)
        uint64_t supply_route;
        for(auto l2_offer_id=selectedPath.begin(); l2_offer_id!=selectedPath.end(); l2_offer_id++) {
          int l2id = *l2_offer_id;
          auto l2offer = offerTable.find(l2id);
          if (l2offer->supply_owner == offer->demand_owner && l2offer->supply_id == offer->demand_id) {
            supply_route = l2offer->route_id;
            break;
          }
        }

        // Now we can create the trade   
        TradeToCreate currentTradeToCreate;
        currentTradeToCreate.setTrade(supply_route, offer->route_id, offer->demand_owner, offer->demand_id, offer->supply_owner, tradeid);
        tradesToCreate.push_back(currentTradeToCreate);
      }
      
      action(
        permission_level{_self, "active"_n},
        _self,
        "tradecreate"_n,
        std::make_tuple(tradesToCreate))
        .send();

      action(
        permission_level{_self, "active"_n},
        stuffContract,
        "stufflock"_n,
        std::make_tuple(stuff_ids))
        .send();

      action(
        permission_level{_self, "active"_n},
        stakeContract,
        "stakelock"_n,
        std::make_tuple(stuff_ids))
        .send();

      action(
        permission_level{_self, "active"_n},
        _self,
        "offersdelall"_n,
        std::make_tuple(stuff_ids))
        .send();
    }

    void markdeliv_m(uint64_t traderoute_id, string shiptracking)
    {
        print("\n==MARKDELIV==\n");
        trade_index gettrades(_self, _self.value);
        auto it = gettrades.find(traderoute_id);
        if (!has_auth(_self) && !has_auth(it->supplier))
        {
            check(false, "Insufficient authority.");
        }
        check(it != gettrades.end(), "Trade route ID does not exist.");

        if (it->delivered.sec_since_epoch() <= 0)
        {
            gettrades.modify(it, _self, [&](auto &m) {
                m.delivered = current_time_point();
            });
        }
        if (shiptracking != "")
        {
            gettrades.modify(it, _self, [&](auto &m) {
                m.delivery_proof = shiptracking;
            });
        }
    }

    void markreceived_m(uint64_t traderoute_id)
    {
        print("\n==MARKRECEIVED==\n");
        trade_index gettrades(_self, _self.value);
        auto it = gettrades.find(traderoute_id);
        if (!has_auth(_self) && !has_auth(it->demandant))
        {
            check(false, "Insufficient authority.");
        }
        check(it != gettrades.end(), "Trade route ID does not exist.");

        gettrades.modify(it, _self, [&](auto &m) {
            if (it->delivered.sec_since_epoch() <= 0)
            {
                m.delivered = current_time_point();
            }
            m.received = current_time_point();
        });
    }
    
    void offer_create_utility(uint64_t supply, uint64_t demand, name creator, time_point_sec expires_at, bool trade_contingent) 
    {
      // Get stuff table via the stuff contract from settings  
      settings_index settings(_self, _self.value);
      auto settings_itr = settings.get();
      name stuffContract = settings_itr.stuffContract;
      stuff_index stuffTable(stuffContract, stuffContract.value);

      // Check supplied and demanded stuff meet various criteria
      auto supply_itr = stuffTable.find(supply);
      auto demand_itr = stuffTable.find(demand);

      // Get the offers table
      offers_index offerTable(_self, _self.value);

      // Track count
      int count;

      // Verify the offer doesn't exist yet
      for (auto itr = offerTable.begin(); itr != offerTable.end(); itr++)
      {
        if (itr->supply_owner == creator && itr->supply_id == supply && itr->demand_owner == demand_itr->account &&itr->demand_id == demand)
        {
          offerrenew_m(creator, itr->offer_id, expires_at);
          return;
        }
        count++;
      }
      
      // Get IDs from seqmngr
      uint64_t routeId = genseqnum(TS_SEQTYPE_OROUTE);
      uint64_t offerid = genseqnum(TS_SEQTYPE_OFFER);

      // Create the offer
      offerTable.emplace(_self, [&](auto &c) {
          c.offer_id = offerid;
          c.route_id = routeId;
          c.supply_owner = creator;
          c.supply_id = supply;
          c.demand_owner = demand_itr->account;
          c.demand_id = demand;
          c.created_at = current_time_point();
          c.expires_at = expires_at;
      });

      action(
        permission_level{_self, "active"_n},
        _self,
        "report"_n,
        std::make_tuple(routeId))
      .send();

      // Ensure more than one offer exists on the offer table
      if (count > 1) {
        // Now we can look for trades
        getpottrades_m(supply, demand, routeId, trade_contingent);
      }
    }

    void offercreate_m(uint64_t supply, uint64_t demand, name creator, time_point_sec expires_at)
    {
        // print("\n==OFFERCREATE==\n");
        
        // Get stuff table via the stuff contract from settings  
        settings_index settings(_self, _self.value);
        auto settings_itr = settings.get();
        name stuffContract = settings_itr.stuffContract;
        stuff_index stuffTable(stuffContract, stuffContract.value);

        // Check supplied and demanded stuff meet various criteria
        auto supply_itr = stuffTable.find(supply);
        auto demand_itr = stuffTable.find(demand);
        check(supply_itr != stuffTable.end(), "Supply ID does not exist.");
        check(demand_itr != stuffTable.end(), "Demand ID does not exist.");
        check(supply_itr->account == creator, "Offer creator must own supply Stuff ID.");
        check(demand_itr->account != creator, "Offer creator cannot own demand Stuff ID.");
        check(supply_itr->status == "active", "Supply Stuff ID must have status equal to active.");
        check(demand_itr->status == "active", "Demand Stuff ID must have status equal to active.");
        check(supply_itr->value >= demand_itr->min_offer_value, "Supply Stuff ID cannot have value less than Demand Stuff IDs Min. Offer Value.");
        check(supply_itr->collateralized == true, "Supply Stuff ID must have collateral equal to true.");
        check(demand_itr->collateralized == true, "Demand Stuff ID must have collateral equal to true.");
        
        // Check expiration is in the future
        check(expires_at > current_time_point(), "Expiration date must be in the future.");
      
        offer_create_utility(supply, demand, creator, expires_at, false);
    }

    void offerdelete_m(name creator, uint64_t offer_id)
    {
        print("\n==OFFERDELETE==\n");
        
        offers_index offersTable(_self, _self.value);

        auto offerIdIndex = offersTable.get_index<"byofferid"_n>();
        auto itr = offerIdIndex.lower_bound(offer_id);
        check(itr != offerIdIndex.end() && itr->offer_id == offer_id, "Offer ID does not exist.");

        check(itr->supply_owner == creator, "Offer creator must be equal to account.");
        while(itr != offerIdIndex.end() && itr->offer_id == offer_id) { 
            // print("Deleting offer ", itr->route_id, "...");
            itr = offerIdIndex.erase(itr);
            // print("....Complete\n");
        }
    }

    void offerrenew_m(name creator, uint64_t offer_id, time_point_sec new_expires_at)
    {
        // print("\n==OFFERRENEW==\n");
        
        offers_index offersTable(_self, _self.value);
        auto index = offersTable.get_index<"bycreator"_n>();
        auto it = index.lower_bound(creator.value);

        check(it->supply_owner == creator, "Offer creator must be equal to account.");
        auto index2 = offersTable.get_index<"byofferid"_n>();
        auto itr = index2.lower_bound(offer_id);
        check(itr != index2.end(), "Offer ID does not exist.");
        check(new_expires_at > current_time_point(), "Expiration date must be in the future.");
        check(new_expires_at > itr->expires_at, "Expiration date must be greater than existing expiration date.");

        settings_index settings(_self, _self.value);
        auto settings_itr = settings.get();
        name stuffContract = settings_itr.stuffContract;
        
        stuff_index stuffTable(stuffContract, stuffContract.value);
        auto supply_itr = stuffTable.find(itr->supply_id);
        auto demand_itr = stuffTable.find(itr->demand_id);
        check(supply_itr->collateralized == true, "Supply Stuff ID must have collateral equal to true.");
        check(demand_itr->collateralized == true, "Demand Stuff ID must have collateral equal to true.");

        time_point_sec oldExp = itr->expires_at;
        
        index2.modify(itr, _self, [&](auto &m) {
            m.expires_at = new_expires_at;
        });

        if (oldExp < current_time_point()) {
          print("Running getpottrades\n");
          // Now we can look for trades
          getpottrades_m(itr->supply_id, itr->demand_id, itr->route_id, false);
        }
    }

    void offersdelall_m(vector<uint64_t> stuff_ids)
    {
        /*
         * No longer fires offerdelete as an inline since we run
         * into dereference errors due to the need to use multiple
         * indexes, one being bysupply the other being byofferid,
         * so when offerdelete deletes an iterator within its own
         * method, this method doesn't know and it's dereferenced
         * in the next iteration of the for loop.
         */

        print("\n==OFFERSDELALL==\n");
        for (auto stuff_id : stuff_ids)
        {
            // print("DELETING OFFERS WITH STUFF ", stuff_id, "\n");
            offers_index getoffers(_self, _self.value);
            
            auto sindex = getoffers.get_index<"bysupply"_n>();
            auto sit = sindex.lower_bound(stuff_id);
            while(sit != sindex.end() && sit->supply_id == stuff_id) {
              // print("DELETING SUPPLY ROUTE", sit->route_id, "\n");
              sit = sindex.erase(sit);
            }

            auto dindex = getoffers.get_index<"bydemand"_n>();
            auto dit = dindex.lower_bound(stuff_id);
            while(dit != dindex.end() && dit->demand_id == stuff_id) {
              // print("DELETING DEMAND ROUTE", dit->route_id, "\n");
              dit = dindex.erase(dit);
            }
        }
    }

    void offersdelexp_m(time_point_sec exp_days)
    {
        print("\n==OFFERSDELEXP==\n");
        offers_index getoffers(_self, _self.value);
        check(exp_days.sec_since_epoch() > 0, "exp_days must be greater than zero.");
        auto itr = getoffers.begin(); 
        while (itr != getoffers.end())
        {
            if (itr->expires_at < current_time_point() && itr->expires_at <= exp_days)
            {
                itr = getoffers.erase(itr);
            }
        }
    }

    void optionaccept_m(uint64_t supply, uint64_t demand, name creator)
    {
        print("\n==OPTIONACCEPT==\n");   
        
        // Get stuff table via the stuff contract from settings  
        settings_index settings(_self, _self.value);
        auto settings_itr = settings.get();
        name stuffContract = settings_itr.stuffContract;
        stuff_index stuffTable(stuffContract, stuffContract.value);

        // Check supplied and demanded stuff meet various criteria
        auto supply_itr = stuffTable.find(supply);
        auto demand_itr = stuffTable.find(demand);
        check(supply_itr != stuffTable.end(), "Supply ID does not exist.");
        check(demand_itr != stuffTable.end(), "Demand ID does not exist.");
        check(supply_itr->account == creator, "Creator must own supply Stuff ID.");
        check(demand_itr->account != creator, "Offer creator cannot own demand Stuff ID.");
        check(supply_itr->status == "active", "Supply stuff must have status equal to active.");
        check(demand_itr->status == "active", "Demand stuff must have status equal to active.");
        check(supply_itr->collateralized == true, "Supply Stuff ID must have collateral equal to true.");
        check(demand_itr->collateralized == true, "Demand Stuff ID must have collateral equal to true.");

        offer_create_utility(supply, demand, creator, current_time_point() + seconds(10), true);
    }

    void setconfig_m(name stuffContract, name stakeContract, name arbContract, name accountsContract)
    {
        print("\n==SETCONFIG==\n");
        settings_index settings_table(_self, _self.value);
        settings new_settings;
        new_settings.stuffContract = stuffContract;
        new_settings.stakeContract = stakeContract;
        new_settings.arbitrationContract = arbContract;
        new_settings.accountsContract = accountsContract;
        settings_table.set(new_settings, _self);
    }

    void stuffaccept_m(uint64_t traderoute_id)
    {
      print("\n==STUFFACCEPT==\n");
      trade_index gettrades(_self, _self.value);
      auto it = gettrades.find(traderoute_id);
      if (!has_auth(_self) && !has_auth(it->demandant))
      {
          check(false, "Insufficient authority.");
      }

      check(it != gettrades.end(), "Trade route ID does not exist.");

      gettrades.modify(it, _self, [&](auto &m) {
          m.accepted = current_time_point();
      });

      vector<uint64_t> traderoute_ids;
      vector<uint64_t> stuff_ids;
      while (it != gettrades.end() && it->route_id == traderoute_id)
      {
          traderoute_ids.push_back(traderoute_id);
          stuff_ids.push_back(it->stuff_id);
          it++;
      }

      settings_index getsettings(_self, _self.value);
      auto setitr = getsettings.get();

      action(
          permission_level{_self, "active"_n},
          setitr.stakeContract,
          "stakeunlock"_n,
          std::make_tuple(stuff_ids))
          .send();

      action(
          permission_level{_self, "active"_n},
          setitr.stuffContract,
          "stuffinactiv"_n,
          std::make_tuple(stuff_ids))
          .send();

      auto itr = gettrades.find(traderoute_id);
      action(
          permission_level{_self, "active"_n},
          setitr.stuffContract,
          "stuffxfer"_n,
          std::make_tuple(itr->supplier, itr->demandant, stuff_ids))
          .send();

      action(
          permission_level{_self, "active"_n},
          _self,
          "trdroutedel"_n,
          std::make_tuple(traderoute_ids))
          .send();
    }

    void tradecreate_m(vector<TradeToCreate> trades)
    {
      print("\n==TRADECREATE==\n");
      
      for (auto trade : trades) {
        uint64_t routeId = genseqnum(TS_SEQTYPE_TROUTE);
        
        trade_index gettrades(_self, _self.value);
        gettrades.emplace(_self, [&](auto &e) {
            e.route_id = routeId;
            e.trade_id = trade.trade_id;
            e.sup_offer_route = trade.sup_offer_route;
            e.dem_offer_route = trade.dem_offer_route;
            e.supplier = trade.supplier;
            e.stuff_id = trade.stuff_id;
            e.demandant = trade.demandant;
            e.created_at = current_time_point();
            e.delivery_proof = "";
        });
        // action(
        //   permission_level{_self, "active"_n},
        //   _self,
        //   "report"_n,
        //   std::make_tuple(routeId))
        // .send();
      }
    }

    void trdroutedel_m(vector<uint64_t> traderoute_id)
    {
        print("\n==TRDROUTEDEL==\n");
        settings_index getsettings(_self, _self.value);
        auto setitr = getsettings.get();

        trade_index gettrades(_self, _self.value);
        disputes_index disputeTable(setitr.arbitrationContract, setitr.arbitrationContract.value);

        for (auto routeItr : traderoute_id)
        {
            print("Route to try to delete ", routeItr);
            auto it = gettrades.find(routeItr);
            check(it != gettrades.end(), "Trade route ID does not exist.");
            
            if (it != gettrades.end())
            {
                // Remove disputes
                for (auto i = disputeTable.begin(); i != disputeTable.end();) {
                  print(i->trade_route_id, " -- ", routeItr, "\n");
                  if (i->trade_route_id == routeItr) {
                    action(
                        permission_level{_self, "active"_n},
                      setitr.arbitrationContract,
                        "disputedel"_n,
                        std::make_tuple(i->dispute_id, i->moderator))
                        .send();
                  }
                  i++;
                }
                // rEMOVE TRADE
                it = gettrades.erase(it);
            }
        }
    }

    void updtdispute_m(uint64_t route_id, uint64_t dispute_id)
    {
        print("\n==UPDTDISPUTE==\n");
        trade_index tradeTable(_self, _self.value);
        auto trade_itr = tradeTable.find(route_id);
        if (trade_itr != tradeTable.end() && trade_itr->route_id == route_id)
        {
            tradeTable.modify(trade_itr, _self, [&](auto &m) {
                m.dispute_id = dispute_id;
            });
        }
    }

    uint64_t genseqnum(const std::string &type)
    {
        uint64_t id = 1;
        std::string retMsg;

        bool bCheck = (type == TS_SEQTYPE_OFFER ||
                      type == TS_SEQTYPE_TRADE ||
                      type == TS_SEQTYPE_OROUTE ||
                      type == TS_SEQTYPE_TROUTE);
        check(bCheck, "Provided type is not a valid table type for this application.\n");

        seqmgr_index seqmgridx(_self, _self.value);
        checksum256 name_sha;
        name_sha = sha256(const_cast<char *>(type.c_str()), type.size());
        auto sha_index = seqmgridx.get_index<"bytype"_n>();
        auto itr = sha_index.find(seqmgr::to_key(name_sha));
        if (itr == sha_index.end())
        {
            id = 1;
            seqmgridx.emplace(_self, [&](auto &seq) {
                seq.pkid = seqmgridx.available_primary_key();
                seq.id = id;
                seq.type = type;
            });
        }
        else
        {
            id = itr->id + 1;
            seqmgridx.modify(*itr, _self, [&](auto &seq) {
                seq.id = id;
            });
        }
        return id;
    }
};
