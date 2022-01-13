
function onProjectileDeath(star, proj)
	star:addAnimation(Animation.new("SMALL_EXPLOSION", proj:getPos()))
	Sounds.playSoundLocal("data/sound/boom2.wav", star, proj:getPos(), 25, 1.0 + Random.randFloat(-0.5, 0.5), false)
end