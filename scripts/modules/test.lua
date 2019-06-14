

package.path = package.path ..';..\\?.lua';

require 'modules.main'


print(table.serialize(CopyScript.common(1,2,3,10)))
--[[ 
结果:
{
    ["map_events"]={
        [1]={["event_baseid"]=7503,["_COMMENT"]="R",},
        [2]={["event_baseid"]=7003,["_COMMENT"]="M",},
        [3]={["event_baseid"]=7012,["_COMMENT"]="M",},
        [4]={["event_baseid"]=7009,["_COMMENT"]="M",},
        [5]={["event_baseid"]=7502,["_COMMENT"]="E",},
        [6]={["event_baseid"]=7501,["_COMMENT"]="R",},
    },
    ["map_baseid"] = 6000
    ["map_baseid"]=5000,
} 
]]

print(Random.randomNormal(3,1))


print(EventScript.test())