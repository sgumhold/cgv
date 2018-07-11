#version 150 compatibility

uniform bool local_viewer = false;
uniform int lights_enabled[gl_MaxLights]; // tells about the state of the lights (whether they are enabled or not)

/**
 * Computes the lighting calculations for a directional light.
 *
 * \param lightSource the light source for which to do the lighting
 * \param lightModelProduct the light product for the light source
 * \param normal the surface normal for the fragment
 * \param ambient the ambient factor in which to write the new ambient factor
 * \param diffuse the diffuse factor in which to write the new diffuse factor
 * \param specular the specular factor in which to write the new specular factor
 */
void computeDirectionalLight(in gl_LightSourceParameters lightSource,
	    					 in gl_LightProducts lightProduct,
     						 in vec3 normal,
     						 inout vec4 ambient,
     						 inout vec4 diffuse,
     						 inout vec4 specular) 
{
	float diffuseFactor = max(dot(normal, normalize(vec3(lightSource.position))), 0.0);
	float specularFactor = 0.0;

	if (diffuseFactor > 0.0)
	{
		specularFactor = max(dot(normal, vec3(lightSource.halfVector)), 0.0);
		specularFactor = pow(specularFactor, gl_FrontMaterial.shininess);
	}

	ambient += lightProduct.ambient;
	diffuse += lightSource.diffuse * diffuseFactor;
	specular += lightProduct.specular * specularFactor;
}

/**
 * Computes the lighting calculations for a directional light in the Phong illumination modell.
 *
 * \param lightSource the light source for which to do the lighting
 * \param lightModelProduct the light product for the light source
 * \param normal the surface normal for the fragment
 * \param ambient the ambient factor in which to write the new ambient factor
 * \param diffuse the diffuse factor in which to write the new diffuse factor
 * \param specular the specular factor in which to write the new specular factor
 */
void computeDirectionalLightPhong(in gl_LightSourceParameters lightSource,
	    					 in gl_LightProducts lightProduct,
     						 in vec3 viewVector,
							 in vec3 normal,
							 inout vec4 ambient,
     						 inout vec4 diffuse,
     						 inout vec4 specular)
{
	vec3 lightVector = normalize(vec3(lightSource.position));
	float diffuseFactor = max(dot(normal, lightVector), 0.0);
	float specularFactor = 0.0;

	if (diffuseFactor > 0.0)
	{
		vec3 reflectVector = 2.0*dot(normal,lightVector)*normal - lightVector;
		specularFactor = max(dot(reflectVector, viewVector), 0.0);
		specularFactor = pow(specularFactor, gl_FrontMaterial.shininess);
	}

	ambient += lightProduct.ambient;
	diffuse += lightSource.diffuse * diffuseFactor;
	specular += lightProduct.specular * specularFactor;
}

/**
 * Computes the lighting calculations for a point light.
 *
 * \param lightSource the light source for which to do the lighting
 * \param lightModelProduct the light product for the light source
 * \param viewVector the viewing vector
 * \param position the position of the fragment in eye coordinate space
 * \param normal the surface normal for the fragment
 * \param ambient the ambient factor in which to write the new ambient factor
 * \param diffuse the diffuse factor in which to write the new diffuse factor
 * \param specular the specular factor in which to write the new specular factor
 */
void computePointLight(in gl_LightSourceParameters lightSource,
   					   in gl_LightProducts lightProduct,
   					   in vec3 viewVector,
   					   in vec3 position,
   					   in vec3 normal,
   					   inout vec4 ambient,
   					   inout vec4 diffuse,
   					   inout vec4 specular)
{
	vec3 lightVec = lightSource.position.xyz / lightSource.position.w - position; // vector from fragment to light source
	float distance = length(lightVec); // distance between surface and light position
	lightVec = normalize(lightVec);

	// attenuation factor
	float attenuation = 1.0 / (lightSource.constantAttenuation +
							   lightSource.linearAttenuation * distance +
							   lightSource.quadraticAttenuation * distance * distance);

	float diffuseFactor = max(dot(normal, lightVec), 0.0);
	float specularFactor = 0.0;

	if (diffuseFactor > 0.0)
	{
		vec3 halfVector = normalize(lightVec + viewVector);
		specularFactor = max(dot(normal, halfVector), 0.0);
		specularFactor = pow(specularFactor, gl_FrontMaterial.shininess);
	}

	ambient += lightProduct.ambient * attenuation;
	diffuse += lightSource.diffuse * diffuseFactor * attenuation;
	specular += lightProduct.specular * specularFactor * attenuation;
}

/**
 * Computes the lighting calculations for a point light.
 *
 * \param lightSource the light source for which to do the lighting
 * \param lightModelProduct the light product for the light source
 * \param viewVector the viewing vector
 * \param position the position of the fragment in eye coordinate space
 * \param normal the surface normal for the fragment
 * \param ambient the ambient factor in which to write the new ambient factor
 * \param diffuse the diffuse factor in which to write the new diffuse factor
 * \param specular the specular factor in which to write the new specular factor
 */
void computePointLightPhong(in gl_LightSourceParameters lightSource,
   					   in gl_LightProducts lightProduct,
   					   in vec3 viewVector,
   					   in vec3 position,
   					   in vec3 normal,
   					   inout vec4 ambient,
   					   inout vec4 diffuse,
   					   inout vec4 specular)
{
	vec3 lightVec = lightSource.position.xyz / lightSource.position.w - position; // vector from fragment to light source
	float distance = length(lightVec); // distance between surface and light position
	lightVec = normalize(lightVec);

	// attenuation factor
	float attenuation = 1.0 / (lightSource.constantAttenuation +
							   lightSource.linearAttenuation * distance +
							   lightSource.quadraticAttenuation * distance * distance);

	float diffuseFactor = max(dot(normal, lightVec), 0.0);
	float specularFactor = 0.0;

	if (diffuseFactor > 0.0)
	{
		vec3 reflectVector = 2.0*dot(normal,lightVec)*normal - lightVec;
		specularFactor = max(dot(reflectVector, viewVector), 0.0);
		specularFactor = pow(specularFactor, gl_FrontMaterial.shininess);
	}

	ambient += lightProduct.ambient * attenuation;
	diffuse += lightSource.diffuse * diffuseFactor * attenuation;
	specular += lightProduct.specular * specularFactor * attenuation;
}

/**
 * Computes the lighting calculations for a spot light.
 *
 * \param lightSource the light source for which to do the lighting
 * \param lightModelProduct the light product for the light source
 * \param viewVector the viewing vector
 * \param position the position of the fragment in eye coordinate space
 * \param normal the surface normal for the fragment
 * \param ambient the ambient factor in which to write the new ambient factor
 * \param diffuse the diffuse factor in which to write the new diffuse factor
 * \param specular the specular factor in which to write the new specular factor
 */
void computeSpotLight(in gl_LightSourceParameters lightSource,
   					  in gl_LightProducts lightProduct,
   					  in vec3 viewVector,
   					  in vec3 position,
   					  in vec3 normal,
   					  inout vec4 ambient,
   					  inout vec4 diffuse,
   					  inout vec4 specular)
{
	vec3 lightVec = lightSource.position.xyz / lightSource.position.w - position; // vector from fragment to light source
	float distance = length(lightVec); // distance between surface and light position
	lightVec = normalize(lightVec);

	// see if point on surface is inside cone of illumination
	float spotDot = dot(-lightVec, normalize(lightSource.spotDirection));
	if (spotDot >= lightSource.spotCosCutoff)
	{ // light adds contribution
		// attenuation factor
		float attenuation = 1.0 / (lightSource.constantAttenuation +
							   lightSource.linearAttenuation * distance +
							   lightSource.quadraticAttenuation * distance * distance);
		// combine the spotlight and distance attenuation
		if (lightSource.spotExponent != 0.0)
			attenuation *= pow(spotDot, lightSource.spotExponent);

		float diffuseFactor = max(0.0, dot(normal, lightVec));
		float specularFactor = 0.0;

		if (diffuseFactor > 0.0)
		{
			vec3 halfVector = normalize(lightVec + viewVector);
			specularFactor = max(0.0, dot(normal, halfVector));
			specularFactor = pow(specularFactor, gl_FrontMaterial.shininess);
		}

		ambient += lightProduct.ambient * attenuation;
		diffuse += lightSource.diffuse * diffuseFactor * attenuation;
		specular += lightProduct.specular * specularFactor * attenuation;
	}
}

/**
 * Computes the lighting calculations for a spot light.
 *
 * \param lightSource the light source for which to do the lighting
 * \param lightModelProduct the light product for the light source
 * \param viewVector the viewing vector
 * \param position the position of the fragment in eye coordinate space
 * \param normal the surface normal for the fragment
 * \param ambient the ambient factor in which to write the new ambient factor
 * \param diffuse the diffuse factor in which to write the new diffuse factor
 * \param specular the specular factor in which to write the new specular factor
 */
void computeSpotLightPhong(in gl_LightSourceParameters lightSource,
   					  in gl_LightProducts lightProduct,
   					  in vec3 viewVector,
   					  in vec3 position,
   					  in vec3 normal,
   					  inout vec4 ambient,
   					  inout vec4 diffuse,
   					  inout vec4 specular)
{
	vec3 lightVec = lightSource.position.xyz / lightSource.position.w - position; // vector from fragment to light source
	float distance = length(lightVec); // distance between surface and light position
	lightVec = normalize(lightVec);

	// see if point on surface is inside cone of illumination
	float spotDot = dot(-lightVec, normalize(lightSource.spotDirection));
	if (spotDot >= lightSource.spotCosCutoff)
	{ // light adds contribution
		// attenuation factor
		float attenuation = 1.0 / (lightSource.constantAttenuation +
							   lightSource.linearAttenuation * distance +
							   lightSource.quadraticAttenuation * distance * distance);
		// combine the spotlight and distance attenuation
		if (lightSource.spotExponent != 0.0)
			attenuation *= pow(spotDot, lightSource.spotExponent);

		float diffuseFactor = max(0.0, dot(normal, lightVec));
		float specularFactor = 0.0;

		if (diffuseFactor > 0.0)
		{
			vec3 reflectVector = 2.0*dot(normal,lightVec)*normal - lightVec;
			specularFactor = max(dot(reflectVector, viewVector), 0.0);
			specularFactor = pow(specularFactor, gl_FrontMaterial.shininess);
		}

		ambient += lightProduct.ambient * attenuation;
		diffuse += lightSource.diffuse * diffuseFactor * attenuation;
		specular += lightProduct.specular * specularFactor * attenuation;
	}
}

/**
 * Does the lighting calculation.
 *
 * \param position the position of the fragment in eye coordinates
 * \param normal the normal of the fragment in eye coordinates
 * \param diffuseMaterial the diffuse material color
 * \return the fragment color resulting from illumination computations
 */
vec4 doTwoSidedLighting(vec3 position, vec3 normal, vec4 diffuseMaterial, int side)
{
	// http://www.opengl.org/sdk/docs/man/xhtml/glNormal.xml
	normal = normalize(normal); // if GL_NORMALIZE is enabled (Page 217); makes no sense here
	// normal = normal * gl_NormalScale; // if GL_RESCALE_NORMAL is enabled (Page 217); makes no sense here

	// http://www.opengl.org/sdk/docs/man/xhtml/glLightModel.xml
	vec3 viewVector = vec3(0.0, 0.0, 1.0); // not local viewer
	if (local_viewer)
		viewVector = -normalize(position); // if GL_LIGHT_MODEL_LOCAL_VIEWER is set to != 0 (Page 222)

	vec4 ambient = vec4(0.0); // ambient color (variant 1) / factor (variant 2)
	vec4 diffuse = vec4(0.0); // diffuse color (variant 1) / factor (variant 2)
	vec4 specular = vec4(0.0); // specular color (variant 1) / factor (variant 2)

	for (int i = 0; i < gl_MaxLights; i++)
	{
		//if (enabled[i] == 1)
		if (lights_enabled[i] == 1)
		{
            if (gl_LightSource[i].position.w == 0.0)
				computeDirectionalLight(gl_LightSource[i], side == 1 ? gl_FrontLightProduct[i] : gl_BackLightProduct[i], normal, ambient, diffuse, specular);
            else if (gl_LightSource[i].spotCutoff == 180.0)
				computePointLight(gl_LightSource[i], side == 1 ? gl_FrontLightProduct[i] : gl_BackLightProduct[i], viewVector, position, normal, ambient, diffuse, specular);
            else
				computeSpotLight(gl_LightSource[i], side == 1 ? gl_FrontLightProduct[i] : gl_BackLightProduct[i], viewVector, position, normal, ambient, diffuse, specular);
        }
	}
	// sceneColor = Ecm + Acm * Acs
	// 'Ecm' == gl_FrontMaterial.emission
	// 'Acm' == gl_FrontMaterial.ambient
	// 'Acs' = ambient color of scene (set through 'glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);')
	return (side == 1 ? gl_FrontLightModelProduct.sceneColor : gl_BackLightModelProduct.sceneColor) + ambient + diffuseMaterial * diffuse + specular;
}

/**
 * Does the lighting calculation.
 *
 * \param position the position of the fragment in eye coordinates
 * \param normal the normal of the fragment in eye coordinates
 * \param diffuseMaterial the diffuse material color
 * \return the fragment color resulting from illumination computations
 */
vec4 doLightingAmb(vec3 position, vec3 normal, vec4 ambientMaterial, vec4 diffuseMaterial)
{
	// http://www.opengl.org/sdk/docs/man/xhtml/glNormal.xml
	normal = normalize(normal); // if GL_NORMALIZE is enabled (Page 217); makes no sense here
	// normal = normal * gl_NormalScale; // if GL_RESCALE_NORMAL is enabled (Page 217); makes no sense here

	// http://www.opengl.org/sdk/docs/man/xhtml/glLightModel.xml
	vec3 viewVector = vec3(0.0, 0.0, 1.0); // not local viewer
	if (local_viewer)
		viewVector = -normalize(position); // if GL_LIGHT_MODEL_LOCAL_VIEWER is set to != 0 (Page 222)

	vec4 ambient = vec4(0.0); // ambient color (variant 1) / factor (variant 2)
	vec4 diffuse = vec4(0.0); // diffuse color (variant 1) / factor (variant 2)
	vec4 specular = vec4(0.0); // specular color (variant 1) / factor (variant 2)

	for (int i = 0; i < gl_MaxLights; i++)
	{
		//if (enabled[i] == 1)
		if (lights_enabled[i] == 1)
		{
            if (gl_LightSource[i].position.w == 0.0)
                computeDirectionalLight(gl_LightSource[i], gl_FrontLightProduct[i], normal, ambient, diffuse, specular);
            else if (gl_LightSource[i].spotCutoff == 180.0)
                computePointLight(gl_LightSource[i], gl_FrontLightProduct[i], viewVector, position, normal, ambient, diffuse, specular);
            else
                computeSpotLight(gl_LightSource[i], gl_FrontLightProduct[i], viewVector, position, normal, ambient, diffuse, specular);
        }
	}

	// sceneColor = Ecm + Acm * Acs
	// 'Ecm' == gl_FrontMaterial.emission
	// 'Acm' == gl_FrontMaterial.ambient
	// 'Acs' = ambient color of scene (set through 'glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);')
	return gl_FrontLightModelProduct.sceneColor + ambientMaterial * ambient + diffuseMaterial * diffuse + specular;
}

vec4 doLighting(vec3 position, vec3 normal, vec4 diffuseMaterial)
{
	// http://www.opengl.org/sdk/docs/man/xhtml/glNormal.xml
	normal = normalize(normal); // if GL_NORMALIZE is enabled (Page 217); makes no sense here
	// normal = normal * gl_NormalScale; // if GL_RESCALE_NORMAL is enabled (Page 217); makes no sense here

	// http://www.opengl.org/sdk/docs/man/xhtml/glLightModel.xml
	vec3 viewVector = vec3(0.0, 0.0, 1.0); // not local viewer
	if (local_viewer)
		viewVector = -normalize(position); // if GL_LIGHT_MODEL_LOCAL_VIEWER is set to != 0 (Page 222)

	vec4 ambient = vec4(0.0); // ambient color (variant 1) / factor (variant 2)
	vec4 diffuse = vec4(0.0); // diffuse color (variant 1) / factor (variant 2)
	vec4 specular = vec4(0.0); // specular color (variant 1) / factor (variant 2)

	for (int i = 0; i < gl_MaxLights; i++)
	{
		//if (enabled[i] == 1)
		if (lights_enabled[i] == 1)
		{
			if (gl_LightSource[i].position.w == 0.0)
				computeDirectionalLight(gl_LightSource[i], gl_FrontLightProduct[i], normal, ambient, diffuse, specular);
			else if (gl_LightSource[i].spotCutoff == 180.0)
				computePointLight(gl_LightSource[i], gl_FrontLightProduct[i], viewVector, position, normal, ambient, diffuse, specular);
			else
				computeSpotLight(gl_LightSource[i], gl_FrontLightProduct[i], viewVector, position, normal, ambient, diffuse, specular);
		}
	}
	// sceneColor = Ecm + Acm * Acs
	// 'Ecm' == gl_FrontMaterial.emission
	// 'Acm' == gl_FrontMaterial.ambient
	// 'Acs' = ambient color of scene (set through 'glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);')
	return 
		//gl_FrontLightModelProduct.sceneColor + 
		ambient + 
		diffuseMaterial * diffuse + specular;
}
/**
 * Does the lighting calculation with the Phong Illumination model.
 *
 * \param position the position of the fragment in eye coordinates
 * \param normal the normal of the fragment in eye coordinates
 * \param diffuseMaterial the diffuse material color
 * \return the fragment color resulting from illumination computations
 */
vec4 doPhongLighting(vec3 position, vec3 normal, vec4 diffuseMaterial)
{
	// http://www.opengl.org/sdk/docs/man/xhtml/glNormal.xml
	normal = normalize(normal); // if GL_NORMALIZE is enabled (Page 217); makes no sense here
	// normal = normal * gl_NormalScale; // if GL_RESCALE_NORMAL is enabled (Page 217); makes no sense here

	// http://www.opengl.org/sdk/docs/man/xhtml/glLightModel.xml
	vec3 viewVector = vec3(0.0, 0.0, 1.0); // not local viewer
	if (local_viewer)
		viewVector = -normalize(position); // if GL_LIGHT_MODEL_LOCAL_VIEWER is set to != 0 (Page 222)

	vec4 ambient = vec4(0.0); // ambient color (variant 1) / factor (variant 2)
	vec4 diffuse = vec4(0.0); // diffuse color (variant 1) / factor (variant 2)
	vec4 specular = vec4(0.0); // specular color (variant 1) / factor (variant 2)

	for (int i = 0; i < gl_MaxLights; i++)
	{
		//if (enabled[i] == 1)
		if (lights_enabled[i] == 1)
		{
            if (gl_LightSource[i].position.w == 0.0)
                computeDirectionalLightPhong(gl_LightSource[i], gl_FrontLightProduct[i], viewVector, normal, ambient, diffuse, specular);
            else if (gl_LightSource[i].spotCutoff == 180.0)
                computePointLightPhong(gl_LightSource[i], gl_FrontLightProduct[i], viewVector, position, normal, ambient, diffuse, specular);
            else
                computeSpotLightPhong(gl_LightSource[i], gl_FrontLightProduct[i], viewVector, position, normal, ambient, diffuse, specular);
        }
	}

	// sceneColor = Ecm + Acm * Acs
	// 'Ecm' == gl_FrontMaterial.emission
	// 'Acm' == gl_FrontMaterial.ambient
	// 'Acs' = ambient color of scene (set through 'glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);')
	return 
		//gl_FrontLightModelProduct.sceneColor + 
		ambient + diffuseMaterial * diffuse + specular;
}