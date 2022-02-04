some_variable = 3^(5*2)


--This is a lua table
config = {
	title = "Engine - Ivan Alexandru",
	fullscreen = false,
	resolution = {
		width = 1280,
		height = 720
	}
}


function  factorial(n)
	i=n;
	result =1;
	while i>1 do
		result = result *i;
		i=i-1
	end
	print(result)
end

factorial(5)
factorial(7)
factorial(3)