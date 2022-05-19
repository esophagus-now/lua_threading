th = require "thing"

function print_a()
    for i = 1,10000 do
        io.write("a");
    end
    io.write("\n")
end

function print_b()
    for i = 1,10000 do
        io.write("b");
    end
    io.write("\n")
end

t = th.run(print_a)
th.run(print_b)
collectgarbage()
t:join()