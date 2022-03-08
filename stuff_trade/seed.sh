#!/usr/bin/env bash

ACCOUNT_PATH="/home/argomory/projects/tradestuff/contracts/contracts/stuff_accts"
STUFF_PATH="/home/argomory/projects/tradestuff/contracts/contracts/stuff_stuff"
STAKE_PATH="/home/argomory/projects/tradestuff/contracts/contracts/stuff_stake"
TRADE_PATH="/home/argomory/projects/tradestuff/contracts/contracts/stuff_trade"
ARBITRATION_PATH="/home/argomory/projects/tradestuff/contracts/contracts/stuff_arb"
TOKEN_PATH="/home/argomory/projects/tradestuff/contracts/contracts/stuff_token"

cleos create account eosio stuff.accts EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
cleos create account eosio stuff.stuff EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
cleos create account eosio stuff.stake EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
cleos create account eosio stuff.trade EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
cleos create account eosio stuff.arb EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
# cleos create account eosio eosio EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active

cleos set contract stuff.accts $ACCOUNT_PATH -p stuff.accts@active
cleos set contract stuff.stuff $STUFF_PATH -p stuff.stuff@active
cleos set contract stuff.stake $STAKE_PATH -p stuff.stake@active
cleos set contract stuff.trade $TRADE_PATH -p stuff.trade@active
cleos set contract stuff.arb $ARBITRATION_PATH -p stuff.arb@active
# cleos set contract eosio $TOKEN_PATH -p eosio@active

cleos set account permission stuff.accts active --add-code
cleos set account permission stuff.stuff active --add-code
cleos set account permission stuff.stake active --add-code
cleos set account permission stuff.trade active --add-code
cleos set account permission stuff.arb active --add-code
cleos set account permission eosio.token active --add-code

cleos push action stuff.stake tokenvalset '[0, 1]' -p stuff.stake@active

cleos push action stuff.accts setconfig '["stuff.stuff", "stuff.trade", "eosio", "stuff.stake", "STUFF", 4]' -p stuff.accts@active
cleos push action stuff.stuff setconfig '["stuff.accts", "stuff.stake", "stuff.trade", "stuff.arb", 10, 5, "STUFF", 4]' -p stuff.stuff@active
cleos push action stuff.stake setconfig '["stuff.stuff", "stuff.accts", "eosio", "stuff.trade", "stuff.arb", "STUFF", 4]' -p stuff.stake@active
cleos push action stuff.trade setconfig '["stuff.stuff", "stuff.stake", "stuff.arb", "stuff.accts"]' -p stuff.trade@active
cleos push action stuff.arb setconfig '[11, 11, 5, 5, 5, 5, 5, 5, "stuff.trade", "stuff.stake", "stuff.stuff", "STUFF", 4]' -p stuff.arb@active

cleos push action stuff.stuff catcreate '["1.0.0", 0]' -p stuff.stuff@active
cleos push action stuff.stuff catcreate '["1.1.0", 1]' -p stuff.stuff@active
cleos push action stuff.stuff condcreate '["cond1"]' -p stuff.stuff@active

cleos create account eosio alex EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
cleos create account eosio cesar EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
cleos create account eosio jayesh EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
cleos create account eosio virginia EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active
cleos create account eosio mod1 EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG EOS84BLRbGbFahNJEpnnJHYCoW9QPbQEk2iHsHGGS6qcVUq9HhutG -p eosio@active

cleos push action stuff.accts acctcreate '["alex", "alex.png", "alex", "123 test ln", "los angeles", "usa", "ca", 90210, "alex@eoslab.io"]' -p alex@active
cleos push action stuff.accts acctcreate '["cesar", "cesar.png", "cesar", "123 test ln", "los angeles", "usa", "ca", 90210, "cesar@eoslab.io"]' -p cesar@active
cleos push action stuff.accts acctcreate '["virginia", "virginia.png", "virginia", "123 test ln", "los angeles", "usa", "ca", 90210, "virginia@eoslab.io"]' -p virginia@active
cleos push action stuff.accts acctcreate '["jayesh", "jayesh.png", "jayesh", "123 test ln", "los angeles", "usa", "ca", 90210, "jayesh@eoslab.io"]' -p jayesh@active

cleos push action stuff.accts acctcreate '["mod1", "mod1.png", "mod1", "123 test ln", "los angeles", "usa", "ca", 90210, "mod1@eoslab.io"]' -p mod1@active
cleos push action stuff.arb classcreate '["Not happy with this trade"]' -p stuff.arb@active
cleos push action stuff.arb modcreate '["mod1"]' -p stuff.arb@active

cleos push action eosio transfer '[ "eosio", "alex", "10000.0000  STUFF", "faucet" ]' -p eosio@active
cleos push action eosio transfer '[ "eosio", "cesar", "10000.0000  STUFF", "faucet" ]' -p eosio@active
cleos push action eosio transfer '[ "eosio", "virginia", "10000.0000  STUFF", "faucet" ]' -p eosio@active
cleos push action eosio transfer '[ "eosio", "jayesh", "10000.0000  STUFF", "faucet" ]' -p eosio@active

cleos push action eosio transfer '["alex", "stuff.stake", "100.0000 STUFF", "0:alex"]' -p alex@active
cleos push action eosio transfer '["cesar", "stuff.stake", "100.0000 STUFF", "0:cesar"]' -p cesar@active
cleos push action eosio transfer '["virginia", "stuff.stake", "100.0000 STUFF", "0:virginia"]' -p virginia@active
cleos push action eosio transfer '["jayesh", "stuff.stake", "100.0000 STUFF", "0:jayesh"]' -p jayesh@active

# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "alex", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p alex@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "cesar", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p cesar@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "virginia", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p virginia@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "jayesh", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p jayesh@active

# cleos push action stuff.trade offercreate '[1, 2, "alex", "2021-12-01T12:00:00"]' -p alex@active
# cleos push action stuff.trade offercreate '[1, 4, "alex", "2021-12-01T12:00:00"]' -p alex@active
# cleos push action stuff.trade offercreate '[2, 3, "cesar", "2021-12-01T12:00:00"]' -p cesar@active
# cleos push action stuff.trade offercreate '[3, 4, "virginia", "2021-12-01T12:00:00"]' -p virginia@active

# cleos push action stuff.stuff qstncreate '["cesar", "Is this new?", 1]' -p cesar@active
# cleos push action stuff.stuff qstncreate '["cesar", "Does this still work?", 3]' -p cesar@active

# cleos push action stuff.stuff flagcreate '["cesar", "stuff", 1]' -p cesar@active
# cleos push action stuff.stuff flagcreate '["cesar", "stuff", 3]' -p cesar@active
# cleos push action stuff.stuff flagcreate '["virginia", "question", 1]' -p virginia@active
# cleos push action stuff.stuff flagcreate '["virginia", "question", 2]' -p virginia@active
# cleos push action stuff.stuff flagcreate '["virginia", "stuff", 1]' -p virginia@active
# cleos push action stuff.stuff flagcreate '["jayesh", "stuff", 1]' -p jayesh@active

# sleep 1
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "alex", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p alex@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "cesar", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p cesar@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "virginia", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p virginia@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "jayesh", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p jayesh@active

# cleos push action stuff.trade offercreate '[5, 6, "alex", "2021-12-01T12:00:00"]' -p alex@active
# cleos push action stuff.trade offercreate '[6, 7, "cesar", "2021-12-01T12:00:00"]' -p cesar@active
# cleos push action stuff.trade offercreate '[7, 8, "virginia", "2021-12-01T12:00:00"]' -p virginia@active

# sleep 1
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "alex", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p alex@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "cesar", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p cesar@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "virginia", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p virginia@active
# cleos push action stuff.stuff stuffcreate '{"status": "active", "owner": "jayesh", "value": 5.0000, "min_offer_value": 4.0000, "title": "placeholder1", "description": "This is a placeholder", "category": 1, "condition": 1, "local": false, "ships": true, "primarymedia": "https://via.placeholder.com/150/0000FF/fff", "addlmedia": [""], "delivery_notes": "I ship within 24 hours", "mature": 0}' -p jayesh@active

# cleos push action stuff.trade offercreate '[9, 10, "alex", "2021-12-01T12:00:00"]' -p alex@active
# cleos push action stuff.trade offercreate '[10, 11, "cesar", "2021-12-01T12:00:00"]' -p cesar@active
# cleos push action stuff.trade offercreate '[11, 12, "virginia", "2021-12-01T12:00:00"]' -p virginia@active

# cleos push action stuff.trade optionaccept '[4, 1, "jayesh", "2021-12-01T12:00:00"]' -p jayesh@active

# cleos push action stuff.arb dispcreate '[1, 1, "alex"]' -p alex@active