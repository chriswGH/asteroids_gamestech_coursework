#include <stdlib.h>
#include "GameUtil.h"
#include "ExtraLife.h"
#include "BoundingShape.h"

ExtraLife::ExtraLife(void) : GameObject("ExtraLife")
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


ExtraLife::~ExtraLife(void)
{
}

void ExtraLife::Render(void)
{
	// Scales everything by SF 0.5
	// glScalef(0.5, 0.5, 0.5);
    // Start drawing filled polygons
    glBegin(GL_POLYGON);
    // Set colour to red
    glColor3f(1.0, 0.0, 0.0);
    // Draw the left side of the heart
    glVertex3f(-1.5, 0.5, 0.0);
    glVertex3f(-2.5, 1.5, 0.0);
    glVertex3f(-3.5, 1.0, 0.0);
    glVertex3f(-3.5, -1.0, 0.0);
    glVertex3f(-2.5, -1.5, 0.0);
    glVertex3f(-1.5, -0.5, 0.0);
    // Draw the right side of the heart
    glVertex3f(1.5, 0.5, 0.0);
    glVertex3f(2.5, 1.5, 0.0);
    glVertex3f(3.5, 1.0, 0.0);
    glVertex3f(3.5, -1.0, 0.0);
    glVertex3f(2.5, -1.5, 0.0);
    glVertex3f(1.5, -0.5, 0.0);
    // End drawing
	glEnd();
	// Enable lighting
	glEnable(GL_LIGHTING);
}

bool ExtraLife::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() == GameObjectType("Asteroid") || o->GetType() == GameObjectType("Bullet")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void ExtraLife::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}