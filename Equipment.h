/* An Item is a thing that can be dropped by monsters
   and picked up by the player */

#ifndef Equipment_h
#define Equipment_h

#include "Item.h"

class Weapon : public Item {
	protected:
		int min_damage;
		int max_damage;
		void init(int min, int max, int _type, char *_name, char *_desc);

	public:
		Weapon(int min, int max, int _type, char *_name, char *_desc);
		Weapon(char *desc);
		int damage();
		void use(Inventory *parent);
};

class Armor : public Item {
	protected:
		int defense;
		void init(int def, int _type, char *_name, char *_desc);
	
	public:
		Armor(int def, int _type, char *_name, char *_desc);
		Armor(char *desc);
		int get_defense() { return defense; }
		void use(Inventory *parent);
};

#endif
