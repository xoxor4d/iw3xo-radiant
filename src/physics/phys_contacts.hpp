#pragma once

namespace physics
{
	void Phys_DumpGroups(const float(*centroid)[3]);
	void Phys_DumpContact(int contact, dContactGeom* geom);
	void Phys_DumpContacts(const ContactList* contacts, const int* group);
	void Phys_ReduceContacts(const ContactList* in, ContactList* out);
	void Phys_CreateJointForEachContact(ContactList* contacts, dxBody* body0, dxBody* body1, dSurfaceParameters* surfParams, PhysWorld worldIndex);
}