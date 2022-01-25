
function onProjectileDeath(star, proj)
	star:addEffectAnimation(EffectAnimation.new(Effect.LIGHTNING, proj:getPos(), 300))
	Sounds.playSoundLocal("data/sound/electric.wav", star, proj:getPos(), 25, 1.0 + Random.randFloat(-0.5, 0.5), false)

	local p = Projectile.new("LIGHTNING_AOE")
	
	p:setPos(proj:getPos())
	p:setAllegiance(proj:getAllegiance())
	
	star:addProjectile(p)
end