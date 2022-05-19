require "swigtest"

t = swigtest.mystruct()
t.x = 5; t.y = 7;
print(swigtest.atan2(t))

print(swigtest.atan2(7,5))