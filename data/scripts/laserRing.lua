
function onProjectileDeath(star, proj)
	local angle = 0.0
	for i = 0, 8 do
		local p = Projectile.new("LASER");
		p:setPos(proj:getPos())
		p:setRotation(angle)
		p:setAllegiance(proj:getAllegiance())
		angle = angle + 45.0
		star:addProjectile(p)
	end
end