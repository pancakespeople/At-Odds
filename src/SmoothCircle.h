#pragma once
#include <SFML/Graphics.hpp>

class SmoothCircle : public sf::Drawable, public sf::Transformable {
public:
	SmoothCircle(float radius = 0) {
		setRadius(radius);
	}

	float getBlur() const { return m_blur; }
	void setBlur(float blur) { m_blur = blur; }

	float getOutlineThickness() const { return m_thickness; }
	void setOutlineThickness(float thickness) { m_thickness = thickness; }

	float getRadius() const { return m_rect.getSize().x / 2.0; }
	void setRadius(float radius) { m_rect.setSize(sf::Vector2f(radius * 2.0f, radius * 2.0f)); }

	sf::Color getColor() const { return m_rect.getFillColor(); }
	void setColor(const sf::Color& color) { m_rect.setFillColor(color); }

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		states.transform *= getTransform();

		sf::Shader* shader = getShader();
		shader->setUniform("size", m_rect.getSize());
		shader->setUniform("blur", m_blur / getRadius() * 100.0f);
		shader->setUniform("thickness", m_thickness / getRadius());
		states.shader = getShader();

		target.draw(m_rect, states);
	}

	static inline const std::string vertexShader = R"(
		#version 130

		out vec2 uv;

		uniform vec2 size;

		void main() {
			// transform the vertex position
			gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
			uv = (gl_Vertex.xy-size/2.0) / size;

			// transform the texture coordinates
			gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

			// forward the vertex color
			gl_FrontColor = gl_Color;
		}
		
	)";

	static inline const std::string fragmentShader = R"(
		#version 130		

		in vec2 uv;		
		uniform float blur;
		uniform float thickness;
		uniform float time;

		void main() {
			gl_FragColor = vec4(smoothstep(0.5, 0.5-blur, length(uv)) - smoothstep(0.5-thickness, 0.5-thickness-blur, length(uv))) * gl_Color;
		}
		
	)";

	static sf::Shader* getShader() {
		static sf::Shader shader;
		static bool loaded = false;

		if (!loaded) {
			shader.loadFromMemory(vertexShader, fragmentShader);
			loaded = true;
		}

		return &shader;
	}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_rect;
		archive & m_blur;
		archive & m_thickness;
	}
	
	sf::RectangleShape m_rect;
	float m_blur = 0.01f;
	float m_thickness = 1.0f;
};