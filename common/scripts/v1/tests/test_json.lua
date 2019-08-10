require('tools/tools')

json = require('tools/json')
local jsonstr = json.encode({ 1, 2, 'fred', {first='mars',second='venus',third='earth地球waed'}, {1,2,3,4,5,'asdsa'} })
print(jsonstr)
local o = json.decode(jsonstr)
table.dump(o)


