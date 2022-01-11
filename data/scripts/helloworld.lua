print('Hello from lua')

for i = 1, 100 do
	print(Random.randInt(0, 10))
	print(Random.randBool())
	print()
end

function coolFunction(num1, num2)
	print("coolFunction outputs: ", num1 + num2)
	return num1 * num2;
end