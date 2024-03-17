#include <stdlib.h>
#include "GameUtil.h"
#include "Shield.h"
#include "BoundingShape.h"

Shield::Shield(void) : GameObject("Shield")
{
	mAngle = rand() % 360;
	mRotation = rand() % 90;
	mPosition.x = rand() / 2;
	mPosition.y = rand() / 2;
	mPosition.z = 0.0;
	mVelocity.x = 10.0 * cos(DEG2RAD * mAngle);
	mVelocity.y = 10.0 * sin(DEG2RAD * mAngle);
	mVelocity.z = 0.0;
}


Shield::~Shield(void)
{
}

void Shield::Render(void)
{
	// Scales everything by SF 0.5
	// glScalef(0.5, 0.5, 0.5);
	// Start drawing filled polygons
	glBegin(GL_POLYGON);
	// Set colour to light blue
	glColor3f(0.6, 0.8, 1.0);
	// Add vertices to draw a diamond
	glVertex3f(0.0, -5.0, 0.0);  // Bottom point
	glVertex3f(-3.0, 0.0, 0.0);   // Left point
	glVertex3f(0.0, 5.0, 0.0);    // Top point
	glVertex3f(3.0, 0.0, 0.0);    // Right point
	// End drawing
	glEnd();
	// Enable lighting
	glEnable(GL_LIGHTING);
}

bool Shield::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() == GameObjectType("Asteroid") || o->GetType() == GameObjectType("Bullet") || o->GetType() == GameObjectType("ExtraLife")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Shield::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}