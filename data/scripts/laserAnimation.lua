
function drawAnimation(renderer, sourcePos, endPos, step)
	local angle = AOMath.angleBetween(sourcePos, endPos)
	local shape = sf.RectangleShape.new()

	shape:setRotation(-angle)
	shape:setPosition(sourcePos)
	shape:setSize(sf.Vector2f.new(AOMath.distance(sourcePos, endPos), 25.0))
	shape:setFillColor(sf.Color.new(255, 0, 0, 255 * (1.0 / step)))

	renderer:draw(shape)
end