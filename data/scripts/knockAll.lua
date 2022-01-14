
function onProjectileDeath(star, proj)
	star:addAnimation(Animation.new("EXPLOSION", proj:getPos()))
	Sounds.playSoundLocal("data/sound/boom1.wav", star, proj:getPos(), 50.0, 0.25, false)

	local spaceships = star:getSpaceships()
	for i, ship in pairs(spaceships) do
		local dist = AOMath.distance(proj:getPos(), ship:getPos())
		local accel = 1000000000.0 / ship:getMass() / (dist * 4.0)
		local angle = AOMath.angleBetween(proj:getPos(), ship:getPos())
		ship:addVelocity(sf.Vector2f.new(math.cos(angle * AOMath.toRadians) * accel, -math.sin(angle * AOMath.toRadians) * accel))
	end
end