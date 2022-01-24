
function onProjectileDeath(star, proj)
	star:addEffectAnimation(EffectAnimation.new(Effect.LIGHTNING, proj:getPos(), 300))
	
	local p = Projectile.new("LIGHTNING_AOE")
	
	p:setPos(proj:getPos())
	p:setAllegiance(proj:getAllegiance())
	
	star:addProjectile(p)
end