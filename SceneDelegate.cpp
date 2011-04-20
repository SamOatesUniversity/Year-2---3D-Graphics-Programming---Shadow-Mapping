//================================================================================================
// Name         : SceneDelegate.cpp
// Author(s)    : Tyrone Davison
// Version      : 1.0
// Copyright    : Tyrone Davison, Teesside University, 2011
// Description  : Model class to provide mesh, shape, and light scene data.
// Instructions : Your work will be marked using the original version of this file.
//================================================================================================

#include <cassert>
#include "SceneDelegate.hpp"
#include "tsl/tsl.hpp"

static float vectorMagnitude(const Float3& v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static Float3 scaleVector(const Float3& v, float s) {
	return Float3(v.x * s, v.y * s, v.z * s);
}

// helper to correct normals and winding order
static void normalizeMesh(bool invertNormals, Mesh* mesh) {

	// unitize normal vectors
	for (std::vector<Float3>::iterator it = mesh->normalArray.begin();
		it != mesh->normalArray.end(); it++) {
			const float length = invertNormals ? -1.f : 1.f * vectorMagnitude(*it);
			*it = scaleVector(*it, 1.f / length);
	}

	// change winding order if necessary
	if (invertNormals) {
		const unsigned int numberOfTriangles = mesh->indexArray.size() / 3;
		for (unsigned int i=0; i<numberOfTriangles; ++i) {
			const unsigned int idx = 3 * i;
			const unsigned int tmp = mesh->indexArray[idx];
			mesh->indexArray[idx] = mesh->indexArray[idx + 1];
			mesh->indexArray[idx + 1] = tmp;
		}
	}
}

SceneDelegate::SceneDelegate(void) {
	const unsigned int numberOfMeshes = 4;

	// use TSL to generate some geometry
	tsl::IndexedMesh meshes[numberOfMeshes];
	tsl::CreateBox( 50, 50, 15, 10, &meshes[0] );
	tsl::CreateCube( 5, 2, &meshes[1] );
	tsl::CreateCone( 3, 5, 10, &meshes[2] );
	tsl::CreateSphere( 3, 10, &meshes[3] );
	bool invertNormals[numberOfMeshes] = {true, false, false, false};

	// convert TSL geometry into framework format
	mesh_.resize(numberOfMeshes);
	for (unsigned int i=0; i<numberOfMeshes; ++i) {
		const unsigned int numberOfTriangles = tsl::ConvertPolygonsToTriangles(&meshes[i]);
		const unsigned int numberOfVertices = meshes[i].vertex_array.size();
		mesh_[i].vertexArray.assign((const Float3*) &meshes[i].vertex_array.front(),
			(const Float3*) &meshes[i].vertex_array.back()+1);
		mesh_[i].normalArray.assign((const Float3*) &meshes[i].normal_array.front(),
			(const Float3*) &meshes[i].normal_array.back()+1);
		mesh_[i].indexArray.assign(&meshes[i].index_array.front(), &meshes[i].index_array.back()+1);
		normalizeMesh(invertNormals[i], &mesh_[i]);
	}

	// initialise shapes
	const unsigned int numberOfShapes = 4;
	const Float3 shapePositions[numberOfShapes] = {
		Float3(0, 0, 7.5f), Float3(-8, 0, 2.5f), Float3(0, 0, 0), Float3(8, 0, 3) };
		shape_.resize(numberOfShapes);
		for (unsigned int i=0; i<numberOfShapes; ++i) {
			shape_[i].meshIndex = i;
			shape_[i].position = shapePositions[i];
		}

		// initialise lights
		const unsigned int numberOfLights = 2;
		light_.resize(numberOfLights);
		for (unsigned int i=0; i<numberOfLights; ++i) {
			light_[i].coneAngle = 1;
			light_[i].direction = Float3(0, 0, -1);
			light_[i].intensity = 1;
		}
		light_[0].intensity = 0.8f;
		light_[1].position = Float3(-10, -10, 1);
		light_[1].direction = Float3(0, 1, 0);
		light_[1].coneAngle = 0.3f;
		light_[1].intensity = 0.5f;

		// provide an opportunity for the animation code to modify shapes and lights
		animate(0);
}

SceneDelegate::~SceneDelegate(void) {
}

void SceneDelegate::animate(float time) {
	shape_[1].rotation.z = cos(time);
	shape_[3].position.z = 3.f + 2.f * abs(cos(0.2f * time) * cos(time));
	light_[0].position = Float3(25.f * cos(time), 25.f * sin(time), 10.f);
	light_[0].direction = scaleVector(light_[0].position,
		-1.f / vectorMagnitude(light_[0].position));
}

Float3 SceneDelegate::worldUpDirection(void) const {
	return Float3(0, 0, 1);
}

unsigned int SceneDelegate::numberOfMeshes(void) const {
	return mesh_.size();
}

void SceneDelegate::getMeshAtIndex(unsigned int i, Mesh* m) const {
	assert(m != 0);
	*m = mesh_[i];
}

unsigned int SceneDelegate::numberOfShapes(void) const {
	return shape_.size();
}

Shape SceneDelegate::shapeAtIndex(unsigned int i) const {
	return shape_[i];
}

unsigned int SceneDelegate::numberOfLights(void) const {
	return light_.size();
}

Light SceneDelegate::lightAtIndex(unsigned int i) const {
	return light_[i];
}
