
function onProjectileDeath(star, proj)
	local spaceships = star:getSpaceships()
	for i, ship in pairs(spaceships) do
		local dist = AOMath.distance(proj:getPos(), ship:getPos())
		if dist < 1000.0 then
			local accel = AOMath.clamp(dist * 2.0, 0.0, 50.0)
			local angle = AOMath.angleBetween(proj:getPos(), ship:getPos()) + 180.0
			ship:addVelocity(sf.Vector2f.new(-math.cos(angle * AOMath.toRadians), -math.sin(angle * AOMath.toRadians) * accel))
		end
	end
end