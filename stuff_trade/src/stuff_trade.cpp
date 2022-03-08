#include <stuff_trade.hpp>
#include <ctime>

ACTION stuff_trade::report(uint64_t id) {
  require_auth(_self);
}

ACTION stuff_trade::setconfig(name stuffContract, name stakeContract, name arbContract, name accountsContract)
{
    require_auth(_self);
    setconfig_m(stuffContract, stakeContract, arbContract, accountsContract);
}

ACTION stuff_trade::offercreate(uint64_t supply, uint64_t demand, name creator, time_point_sec expires_at)
{
    // Authority check
    if (!has_auth(_self) && !has_auth(creator))
    {
        check(false, "Insufficient authority.");
    }
    offercreate_m(supply, demand, creator, expires_at);
}

ACTION stuff_trade::optionaccept(uint64_t supply, uint64_t demand, name creator)
{
    // Authority check
    if (!has_auth(_self) && !has_auth(creator))
    {
        check(false, "Insufficient authority.");
    }
    optionaccept_m(supply, demand, creator);
}

ACTION stuff_trade::getpottrades(uint64_t supply, uint64_t demand, uint64_t routeId, bool trade_contingent)
{
  // Authority check
  settings_index settings(_self, _self.value);
  auto settings_itr = settings.get();
  if (!has_auth(_self) && !has_auth(settings_itr.stuffContract) && !has_auth(settings_itr.stakeContract))
  {
      check(false, "Insufficient authority.");
  }

  // Run method
  getpottrades_m(supply, demand, routeId, trade_contingent);
}

ACTION stuff_trade::offerrenew(name creator, uint64_t offer_id, time_point_sec new_expires_at)
{
    if (!has_auth(_self) && !has_auth(creator))
    {
        check(false, "Insufficient authority.");
    }
    offerrenew_m(creator, offer_id, new_expires_at);
}

ACTION stuff_trade::offerdelete(name creator, uint64_t offer_id)
{
    if (!has_auth(_self) && !has_auth(creator))
    {
        check(false, "Insufficient authority.");
    }
    offerdelete_m(creator, offer_id);
}

ACTION stuff_trade::offersdelall(vector<uint64_t> stuff_ids)
{
    settings_index getsettings(_self, _self.value);
    auto setitr = getsettings.get();
    if (!has_auth(_self) && !has_auth(setitr.stuffContract) && !has_auth(setitr.accountsContract))
    {
        check(false, "Insufficient authority.");
    }

    offersdelall_m(stuff_ids);
}

ACTION stuff_trade::offersdelexp(time_point_sec exp_days)
{
    require_auth(_self);
    offersdelexp_m(exp_days);
}

ACTION stuff_trade::tradecreate(vector<TradeToCreate> trades)
{
    require_auth(_self);
    tradecreate_m(trades);
}

ACTION stuff_trade::updtdispute(uint64_t route_id, uint64_t dispute_id)
{
    settings_index getsettings(_self, _self.value);
    auto setitr = getsettings.get();
    require_auth(setitr.arbitrationContract);
    updtdispute_m(route_id, dispute_id);
}

ACTION stuff_trade::markdeliv(uint64_t traderoute_id, string shiptracking)
{
    markdeliv_m(traderoute_id, shiptracking);
}

ACTION stuff_trade::markreceived(uint64_t traderoute_id)
{
    markreceived_m(traderoute_id);
}

ACTION stuff_trade::stuffaccept(uint64_t traderoute_id)
{
    stuffaccept_m(traderoute_id);
}

ACTION stuff_trade::trdroutedel(vector<uint64_t> traderoute_id)
{
    settings_index getsettings(_self, _self.value);
    auto setitr = getsettings.get();

    if (!has_auth(_self) && !has_auth(setitr.arbitrationContract))
    {
        check(false, "Insufficient authority.");
    }

    trdroutedel_m(traderoute_id);
}
