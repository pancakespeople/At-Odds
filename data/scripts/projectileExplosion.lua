
function onUnitDeath(star, unit)
	for i = 0, 25 do
		local randAngle = Random.randFloat(0, 360)
		newProj = Projectile.new("FLAK")
		newProj:setPos(unit:getPos())
		newProj:setRotation(randAngle)
		newProj:setAllegiance(unit:getAllegiance())
		star:addProjectile(newProj)
	end
end