
function onProjectileDeath(star, proj)
	star:addAnimation(Animation.new("LIGHTNING", proj:getPos()))
	local p = Projectile.new("LIGHTNING_AOE")
	p:setPos(proj:getPos())
	p:setAllegiance(proj:getAllegiance())
	star:addProjectile(p)
end