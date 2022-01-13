
function onProjectileDeath(star, proj)
	star:addAnimation(Animation.new("EXPLOSION", proj:getPos()))
	Sounds.playSoundLocal("data/sound/boom1.wav", star, proj:getPos(), 25, 1.0 + Random.randFloat(-0.5, 0.5), false)
end