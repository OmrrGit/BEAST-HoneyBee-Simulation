// Vim users: for increased viewing pleasure :set ts=4

#include "animat.h"
#include "sensor.h"
#include "population.h"
#include "beeutilfunc.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace BEAST;

#define SWV(from, to, sqr) shortestWrappedVec(from, to, GetWorld().GetWidth(), GetWorld().GetHeight(), sqr)

double worker_popA = 8;
int forager_popA = 0;
double nurse_popA = worker_popA;

double nectarA = 8000;
double nectarB = 8000;

double worker_popB = 8;
int forager_popB = 0;
double nurse_popB = worker_popB;

Vector2D HiveALoc = Vector2D(100.0, 100.0);
Vector2D HiveBLoc = Vector2D(700.0, 500.0);
Vector2D MatingLoc = Vector2D(400.0, 300.0);

int WorkerRoleDecisionA(int age, int role, int forager_age) {
	
	double prob;
	double roll = randomDouble(0, 1);
	int new_role = role;

	double forager_alpha = 0.4;
	double forager_sigma = 1.25;
	
	double temp_den = forager_age + age;
	double temp_sig = forager_popA / worker_popA;
	prob = forager_alpha * (age / temp_den) * (1 - (forager_sigma * (forager_popA / worker_popA)));

	if (roll < prob) {
		new_role = 1;
		forager_popA += 1;
		nurse_popA -= 1;
	}

	return new_role;
}

int WorkerRoleDecisionB(int age, int role, int forager_age) {
	
	double prob;
	double roll = randomDouble(0, 1);
	int new_role = role;

	double forager_alpha = 0.4;
	double forager_sigma = 1.5;
	
	double temp_den = forager_age + age;
	prob = forager_alpha * (age / temp_den) * (1 - (forager_sigma * (forager_popB / worker_popB)));

	if (roll < prob) {
		new_role = 1;
		forager_popB += 1;
		nurse_popB -= 1;
	}

	return new_role;
}

bool DroneLeaveDecision(int age) {

	bool leave = false;
	double prob;
	double roll = randomDouble(0, 1);
	
	int leave_age = randomInt(500, 650);
	int limit = -1;

	if (leave_age <= age) {
		limit = 1;
	}

	prob = 0.1 * limit;
	if (roll < prob) {
		leave = true;
	}

	return leave;
}

bool QueenLeaveDecision(int age) {

	bool leave = false;
	double prob;
	double roll = randomDouble(0, 1);
	
	int leave_age = 500;
	int limit = -1;

	if (leave_age <= age) {
		limit = 1;
	}

	prob = 0.3 * limit;
	if (roll < prob) {
		leave = true;
	}

	return leave;
}

Vector2D WanderCalc(double circDist, double circRad, double theta, Vector2D vel) {

	Vector2D sphere = circDist * vel.GetNormalised();
	//Vector2D displacement = circDist * vel.GetNormalised();
	//displacement.SetAngle(vel.GetAngle() - theta);
	Vector2D displacement = Vector2D(0.0, 1.0, circRad, vel.GetAngle() - theta);
	Vector2D wanderForce = sphere + displacement;

	return wanderForce;
}

Vector2D SeekingCalc(Vector2D target_path, double maxSpeed, Vector2D vel) {

	Vector2D steer_vel = maxSpeed * (target_path.GetNormalised());
	Vector2D seekForce = steer_vel - vel;

	return seekForce;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration for bees
class HoneyBee;

// Forward declaration for worker role
class WorkerBeeA;
class WorkerBeeB;

// Forward declaration for queen role
class QueenBee;
class QueenBeeA;
class QueenBeeB;

class Nectar : public WorldObject
{
public:
	int availability;
	Nectar()
	{
		This.Radius = 6.0f;
		This.SetColour(ColourPalette[COLOUR_GREEN]);
		This.InitRandom = true;

		This.availability = 3040;
	}
	virtual ~Nectar(){}

	virtual void Update()
	{
		This.availability += 1;
	}

	int Foraged()
	{
		if (This.availability >= 60) {
			This.availability -= 60;
			return 60;
		} else {
			int return_val = This.availability;

			This.Location = This.MyWorld->RandomLocation();
			This.availability = 3040;
			
			return return_val;
		};
	}
};

class HiveA : public WorldObject
{
public:
	int queen_pher;
	int nectar;
	HiveA(Vector2D l = HiveALoc):WorldObject(l, 0.0, 20.0)
	{
		This.SetColour(ColourPalette[COLOUR_ORANGE]);
		
		This.nectar = 10000;
		This.queen_pher = 500;
	}
	virtual ~HiveA(){}

	virtual void Update()
	{
		This.queen_pher -= 1;
	}

	void ForageReturn(int new_nectar)
	{
		nectarA += new_nectar;
		This.nectar = nectarA;
	}

	virtual void OnCollision(WorldObject* obj)
	{
		QueenBeeA* queen;
		HoneyBee* bee;

		if (IsKindOf(obj, bee)) {
			This.nectar -= 1;

			nectarA -= 1;
			This.nectar = nectarA;
		}

		if (IsKindOf(obj, queen)) {
			This.queen_pher += 5;
			This.queen_pher = bound<double>(0, 500, This.queen_pher);
		}
	}
	
};

class HiveB : public WorldObject
{
public:
	int queen_pher;
	int nectar;
	HiveB(Vector2D l = HiveBLoc):WorldObject(l, 0.0, 20.0)
	{
		This.SetColour(ColourPalette[COLOUR_ORANGE]);
		
		This.nectar = nectarB;
		This.queen_pher = 500;
	}
	virtual ~HiveB(){}

	virtual void Update()
	{
		This.queen_pher -= 1;
	}

	void ForageReturn(int new_nectar)
	{
		nectarB += new_nectar;
		This.nectar = nectarB;
	}

	virtual void OnCollision(WorldObject* obj)
	{
		QueenBeeB* queen;
		HoneyBee* bee;

		if (IsKindOf(obj, bee)) {
			nectarB -= 1;
			This.nectar = nectarB;
		}

		if (IsKindOf(obj, queen)) {
			This.queen_pher += 5;
			This.queen_pher = bound<double>(0, 500, This.queen_pher);
		}
	}
};

class MatingArea : public WorldObject
{
public:
	MatingArea(Vector2D l = MatingLoc):WorldObject(l, 0.0, 10.0)
	{
		This.SetColour(ColourPalette[COLOUR_RED]);
	}
	virtual ~MatingArea(){}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DeathProb(int age, double op_age, double hungerLevel, double nectar) {

	bool death = false;
	double prob;
	double roll = randomDouble(0, 1);

	double a = 0.05;
	double k = 5000;
	
	if (age > op_age) {
		prob = a * (1 +(k / ( k + nectar))) * (sqr(age - op_age) / sqr(op_age)) * (1 + ((700 - hungerLevel) / 700));

		if (roll < prob) {
			death = true;
		}	
	}

	return death;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class HoneyBee : public Animat
{
public:
	HoneyBee()
	{
		This.SetMinSpeed(30.0);
		This.SetMaxSpeed(100.0);
		
		// wander beahvioural values
		This.circDist = 70.0;
        This.circRad = 50.0;
        This.rotMax = 0.4*PI;

		// characteristics and properties
		This.age = 0;
		This.hungerLevel = 700;
		This.leave = false;

		//collision values
		This.collided = false;
		This.col_timer = 0;

		//colony boolean for death decision
		This.colonyA = true;
	}

protected:
	bool collided;
	int col_timer;

    double hungerLevel;
    int age;
	bool leave;
	Vector2D hiveLOC;

	double circDist;
    double circRad;
    double rotMax;
	double theta;

	Vector2D maintainedVelocity;

	bool colonyA;
	double optimalAge;
};

// Forward declaration for drone role
class DroneBee;
class DroneBeeA;
class DroneBeeB;

// QueenBee is an inherited Class of HoneyBee representing reproductive females in the colony

class QueenBee : public HoneyBee
{
public:
	QueenBee()
	{
		// queen specific charcteristics
		This.noMates = 0;
		This.leave = false;
		This.optimalAge = 6000;
	}

	void DeathReset() {
		This.noMates = 0;
		This.age = 0;
		This.hungerLevel = 0;
		This.leave = false;
		This.SetLocation(This.hiveLOC);
	}

	virtual void Control()
	{
		This.age += 1;
		This.hungerLevel -= 1;
		double sqr;
		This.theta += randomDouble(-rotMax, rotMax);
		This.theta = bound<double>(0, 2*PI, This.theta);

		Vector2D wanderForce = WanderCalc(This.circDist, This.circRad, This.theta, This.GetVelocity());

		if (This.leave == false && This.noMates == 0) {

			if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
			else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

			if (This.collided == false) {

				Vector2D target_path = SWV(This.GetLocation(), hiveLOC, sqr);
				Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
				Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);

				This.maintainedVelocity = seekForce;

			} else {
				Vector2D newVelocity = This.GetVelocity() + (This.maintainedVelocity) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);
			}

			This.leave = QueenLeaveDecision(This.age);
		}

		if (This.leave == true && This.noMates < 2) {
			
			if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
			else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

			if (This.collided == false) {

				Vector2D target_path = SWV(This.GetLocation(), MatingLoc, sqr);
				Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
				Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);

				This.maintainedVelocity = seekForce;

			} else {
				Vector2D newVelocity = This.GetVelocity() + (This.maintainedVelocity) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);
			}

		} else {

			if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
			else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

			if (This.collided == false) {

				Vector2D target_path = SWV(This.GetLocation(), This.hiveLOC, sqr);
				Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
				Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);

				This.maintainedVelocity = seekForce;

			} else {
				Vector2D newVelocity = This.GetVelocity() + (This.maintainedVelocity) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);
			}

		}

		double hive_nectar;

		if (This.colonyA == true) {hive_nectar = nectarA;}
		else {hive_nectar = nectarB;}

		if (DeathProb(This.age, This.optimalAge, This.hungerLevel, hive_nectar) == true) {
			This.DeathReset();
		}

	}

protected:
	int noMates;
};


class QueenBeeA : public QueenBee
{
public:
	QueenBeeA()
	{
		This.hiveLOC = HiveALoc;
		This.SetStartLocation(HiveALoc);
	}

	virtual void OnCollision(WorldObject* obj)
	{
		DroneBeeB* drone;
		HiveA* hive;
		MatingArea* ma;

		if (IsKindOf(obj, drone)) {
			This.noMates += 1;
		}
		
		if (IsKindOf(obj, hive)) {
			This.hungerLevel = 700;

			This.collided = true;
			This.col_timer = 3;
		}
		
		if (IsKindOf(obj, ma)) {
			This.collided = true;
			This.col_timer = 2;
		}

		Animat::OnCollision(obj);
	}

};

class QueenBeeB : public QueenBee
{
public:
	QueenBeeB()
	{
		This.hiveLOC = HiveBLoc;
		This.SetStartLocation(HiveBLoc);
		This.colonyA = false;
	}

	virtual void OnCollision(WorldObject* obj)
	{
		DroneBeeA* drone;
		HiveB* hive;
		MatingArea* ma;

		if (IsKindOf(obj, drone)) {
			This.noMates += 1;
		}
		
		if (IsKindOf(obj, hive)) {
			This.hungerLevel = 700;

			This.collided = true;
			This.col_timer = 3;
		}

		if (IsKindOf(obj, ma)) {
			This.collided = true;
			This.col_timer = 2;
		}

		Animat::OnCollision(obj);
	}

};

class DroneBee : public HoneyBee
{
public:
	DroneBee()
	{
		// drone specific charcteristics
		This.leave = false;
		This.optimalAge = 1500;
	}

	void DeathReset() {
		This.age = 0;
		This.hungerLevel = 0;
		This.leave = false;
		This.SetLocation(This.hiveLOC);
	}

	virtual void Control()
	{
		This.age += 1;
		This.hungerLevel -= 1;
		double sqr;
		This.theta += randomDouble(-rotMax, rotMax);
		This.theta = bound<double>(0, 2*PI, This.theta);

		Vector2D wanderForce = WanderCalc(This.circDist, This.circRad, This.theta, This.GetVelocity());

		if (This.leave == false) {

			if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
			else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

			if (This.collided == false) {

				Vector2D target_path = SWV(This.GetLocation(), This.hiveLOC, sqr);
				Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
				Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);

				This.maintainedVelocity = seekForce;

			} else {
				Vector2D newVelocity = This.GetVelocity() + (This.maintainedVelocity) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);
			}

			This.leave = DroneLeaveDecision(This.age);
		} else {

			if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
			else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

			if (This.collided == false) {

				Vector2D target_path = SWV(This.GetLocation(), MatingLoc, sqr);
				Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
				Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);

				This.maintainedVelocity = seekForce;

			} else {
				Vector2D newVelocity = This.GetVelocity() + (This.maintainedVelocity) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);
			}
		}
		double hive_nectar;

		if (This.colonyA == true) {hive_nectar = nectarA;}
		else {hive_nectar = nectarB;}

		if (DeathProb(This.age, This.optimalAge, This.hungerLevel, hive_nectar) == true) {
			This.DeathReset();
		}
	}

protected:
	bool leave;
};

class DroneBeeA : public DroneBee
{
public:
	DroneBeeA()
	{
		This.hiveLOC = HiveALoc;
		This.SetStartLocation(HiveALoc);
	}

	virtual void OnCollision(WorldObject* obj)
	{
		QueenBeeB* queen;
		HiveA* hive;
		MatingArea* ma;

		if (IsKindOf(obj, queen)) {
			This.DeathReset();
		}

		if (IsKindOf(obj, hive)) {
			This.hungerLevel = 700;

			This.collided = true;
			This.col_timer = 3;
		}

		if (IsKindOf(obj, ma)) {
			This.collided = true;
			This.col_timer = 2;
		}

		Animat::OnCollision(obj);
	}

};

class DroneBeeB : public DroneBee
{
public:
	DroneBeeB()
	{
		This.hiveLOC = HiveBLoc;
		This.SetStartLocation(HiveBLoc);
		This.colonyA = false;
	}

	virtual void OnCollision(WorldObject* obj)
	{
		QueenBeeA* queen;
		HiveB* hive;
		MatingArea* ma;

		if (IsKindOf(obj, queen)) {
			This.DeathReset();
		}

		if (IsKindOf(obj, hive)) {
			This.hungerLevel = 700;

			This.collided = true;
			This.col_timer = 3;
		}

		if (IsKindOf(obj, ma)) {
			This.collided = true;
			This.col_timer = 2;
		}

		Animat::OnCollision(obj);
	}

};

// WorkerBeeA and WorkerBeeB are inherited Classes of HoneyBee representing nonreproductive females in the colony
// that take on the majority of the work for their hive

class WorkerBeeB : public HoneyBee
{
public:
	WorkerBeeB()
	{
		This.Add("left", ProximitySensor<Nectar>(PI/2, 30.0, -1.4));
		This.Add("right", ProximitySensor<Nectar>(PI/2, 30.0, 1.4));

		This.forager_age = 1000;
		This.optimalAge = 3500;

		This.nectarCollected = 0;
		This.depositTimer = 0;

		This.role = 0;

		This.SetStartLocation(HiveBLoc);
		This.hiveLOC = HiveBLoc;
	}

	void DeathReset() {
		This.age = 0;
		This.hungerLevel = 0;

		This.role = 0;
		This.nectarCollected = 0;
		This.depositTimer = 0;
		
		forager_popB -= 1;
		nurse_popB -= 1;

		This.SetLocation(This.hiveLOC);
	}

	virtual void Control()
	{	
		This.age += 1;
		This.hungerLevel -= 1;
		double sqr;
		This.theta += randomDouble(-rotMax, rotMax);
		This.theta = bound<double>(0, 2*PI, This.theta);

		Vector2D wanderForce = WanderCalc(This.circDist, This.circRad, This.theta, This.GetVelocity());

		if (This.role != 1) {
			This.role = WorkerRoleDecisionB(This.age, This.role, This.forager_age);
		}

		if (This.role == 0) {
			// Nurse and GuardRole (role = 0)
			if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
			else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

			if (This.collided == false) {

				Vector2D target_path = SWV(This.GetLocation(), This.hiveLOC, sqr);
				Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
				Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);

				This.maintainedVelocity = seekForce;

			} else {
				Vector2D newVelocity = This.GetVelocity() + (This.maintainedVelocity) + (0.75 * wanderForce);

				This.SetOrientation(newVelocity.GetAngle());
				This.SetVelocity(newVelocity);
			}
		} else {
			// Forager Role (role = 1)
			// WorkerBeeB will implement much simpler foraging that moves a round randomly until a source is found

			if (This.nectarCollected == 0) {

				double left = This.Sensors["left"]->GetOutput();
				double right = This.Sensors["right"]->GetOutput();

				if (left == 0 && right == 0) {
					Vector2D newVelocity = This.GetVelocity() + wanderForce;
					This.SetOrientation(newVelocity.GetAngle());
					This.SetVelocity(newVelocity);
					
				} else {
					This.Controls["left"] = 1*right;
					This.Controls["right"] = 1*left;

					This.AddVelocity(This.GetVelocity());
				}
			} else {
				if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
				else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

				if (This.collided == false || This.depositTimer > 0) {

					Vector2D target_path = SWV(This.GetLocation(), This.hiveLOC, sqr);
					Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
					Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

					This.SetOrientation(newVelocity.GetAngle());
					This.SetVelocity(newVelocity);

				}
				This.depositTimer -= 1;
				This.depositTimer = bound<int>(0, 10, This.depositTimer);
			}
			
		}

		if (DeathProb(This.age, This.optimalAge, This.hungerLevel, nectarB) == true) {
			This.DeathReset();
		}
		
	}

	void Attacked(int role)
	{
		if (role == 0){This.DeathReset();}
	}

	virtual void OnCollision(WorldObject* obj)
	{
		HiveB* hive;
		Nectar* nectar;

		if (IsKindOf(obj, hive)) {
			This.hungerLevel = 700;
			
			if (This.nectarCollected > 0) {
				hive->ForageReturn(This.nectarCollected);
				This.nectarCollected = 0;
				This.depositTimer = 7;
			}

			This.collided = true;
			This.col_timer = 3;
		}

		if (IsKindOf(obj, nectar)) {

			int nectar_return = nectar->Foraged();

			if (nectar_return >= 60) {
				This.hungerLevel += 5;
				This.hungerLevel = bound<double>(0, 1000, This.hungerLevel);

				This.nectarCollected += nectar_return - 5;	
			} else {
				This.nectarCollected += nectar_return;
			}
		}

		Animat::OnCollision(obj);
	}

protected:
	int role;
	int depositTimer;
	int nectarCollected;
	
	double guard_age;
	double forager_age;
};

class WorkerBeeA : public HoneyBee
{
public:
	WorkerBeeA()
	{
		This.Add("left", ProximitySensor<Nectar>(PI/2, 30.0, -1.4));
		This.Add("right", ProximitySensor<Nectar>(PI/2, 30.0, 1.4));
		This.Add("front", ProximitySensor<Nectar>(TWOPI, 30.0, 0.0));

		This.Add("enemy", ProximitySensor<WorkerBeeB>(TWOPI, 50.0, 0.0));
		This.Add("enemy angle", NearestAngleSensor<WorkerBeeB>());
		
		This.forager_age = 1000;
		This.optimalAge = 3500;

		This.nectarCollected = 0;

		This.role = 0;
		This.onlooker = true;
		This.locationFound = false;
		This.danced = false;

		This.depositTimer = 0;
		This.soloDanceTimer = 0;

		This.SetStartLocation(HiveALoc);
		This.hiveLOC = HiveALoc;
		
	}

	void DeathReset() {
		This.age = 0;
		This.hungerLevel = 0;

		This.role = 0;
		This.nectarCollected = 0;
		This.onlooker = true;
		This.locationFound = false;
		This.danced = false;
		This.depositTimer = 0;
		This.soloDanceTimer = 0;
		This.nectarLoc = Vector2D(0, 0);

		forager_popA -= 1;
		nurse_popA -= 1;

		This.SetLocation(This.hiveLOC);
	}

	virtual void Control()
	{	
		This.age += 1;
		This.hungerLevel -= 1;
		double sqr;
		This.theta += randomDouble(-rotMax, rotMax);
		This.theta = bound<double>(0, 2*PI, This.theta);

		Vector2D wanderForce = WanderCalc(This.circDist, This.circRad, This.theta, This.GetVelocity());

		if (This.role != 1) {

			This.role = WorkerRoleDecisionA(This.age, This.role, This.forager_age);

			if (This.role == 1 && forager_popA % 3 == 1) {
				This.onlooker = false;
			}
		}

		if (This.role == 0) {
			// Nurse Role (role = 0)

			if (This.Sensors["enemy"]->GetOutput() == 0){

				Vector2D target_path = SWV(This.GetLocation(), This.hiveLOC, sqr);
				Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());

				if (target_path.GetLengthSquared() >= 900) {

					Vector2D newVelocity = This.GetVelocity() + (0.5 * seekForce) + (wanderForce);

					This.SetOrientation(newVelocity.GetAngle());
					This.SetVelocity(newVelocity);

				} else {
					
					Vector2D newVelocity = This.GetVelocity() - (0.5 * seekForce) + (wanderForce);

					This.SetOrientation(newVelocity.GetAngle());
					This.SetVelocity(newVelocity);
				}

			} else {
				
				double output = This.Sensors["enemy angle"]->GetOutput();
				This.Controls["right"] = 0.5 - (output > 0.0 ? output : 0.0);
				This.Controls["left"] = 0.5 + (output < 0.0 ? output : 0.0);

				This.SetVelocity(5 * This.GetVelocity());

			}
		} else {

			// Forager Role (role = 1)
			// WorkerBeeA will implement more complex foraging that implements several foraging roles and basic memory

			if (This.onlooker == true) {

				// Onlooker Forager
				if (This.locationFound == false || This.nectarCollected > 0 || depositTimer > 0) {

					if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
					else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

					if (This.collided == false) {

						Vector2D target_path = SWV(This.GetLocation(), This.hiveLOC, sqr);
						Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
						Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

						This.SetOrientation(newVelocity.GetAngle());
						This.SetVelocity(newVelocity);

						This.maintainedVelocity = seekForce;

					} else {
						Vector2D newVelocity = This.GetVelocity() + (This.maintainedVelocity) + (0.75 * wanderForce);

						This.SetOrientation(newVelocity.GetAngle());
						This.SetVelocity(newVelocity);
					}

					This.depositTimer -= 1;
					This.depositTimer = bound<int>(0, 10, This.depositTimer);

				} else {

					Vector2D target_path = SWV(This.GetLocation(), This.nectarLoc, sqr);

					Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
					Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

					This.SetOrientation(newVelocity.GetAngle());
					This.SetVelocity(newVelocity);

					double front = This.Sensors["front"]->GetOutput();

					if ((target_path.GetLengthSquared() < 5) && front == 0) {
						This.locationFound = false;
						This.danced = false;
					}
					
				}
			} else {
				
				double left = This.Sensors["left"]->GetOutput();
				double right = This.Sensors["right"]->GetOutput();

				if (This.locationFound == false) {

					// Scout Forager

					if (left == 0 && right == 0) {
						Vector2D newVelocity = This.GetVelocity() + wanderForce;
						This.SetOrientation(newVelocity.GetAngle());
						This.SetVelocity(newVelocity);
					} else {
						This.Controls["left"] = 1*right;
						This.Controls["right"] = 1*left;

						This.AddVelocity(This.GetVelocity());
					}

				} else {

					// Employed Forager

					if (This.nectarCollected == 0 && This.danced == true && depositTimer == 0) {

						Vector2D target_path = SWV(This.GetLocation(), This.nectarLoc, sqr);

						Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
						Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

						This.SetOrientation(newVelocity.GetAngle());
						This.SetVelocity(newVelocity);

						double front = This.Sensors["front"]->GetOutput();

						if ((target_path.GetLengthSquared() < 5) && front == 0) {
							This.locationFound = false;
							This.danced = false;
						}

					} else {

						Vector2D target_path = SWV(This.GetLocation(), This.hiveLOC, sqr);

						if (This.danced == false && This.soloDanceTimer != 0){

							if (This.collided == true && This.col_timer != 0) {This.col_timer -= 1;}
							else if (This.collided == true && This.col_timer == 0) {This.collided = false;}

							if (This.collided == false) {

								Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
								Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

								This.SetOrientation(newVelocity.GetAngle());
								This.SetVelocity(newVelocity);

								This.maintainedVelocity = seekForce;

							}  else {
								Vector2D newVelocity = This.GetVelocity() + (This.maintainedVelocity) + (0.75 * wanderForce);

								This.SetOrientation(newVelocity.GetAngle());
								This.SetVelocity(newVelocity);
							}

							This.soloDanceTimer -= 1;
							This.soloDanceTimer = bound<int>(0, 325, This.soloDanceTimer);

						} else {

							Vector2D seekForce = SeekingCalc(target_path, This.GetMaxSpeed(), This.GetVelocity());
							Vector2D newVelocity = This.GetVelocity() + (seekForce) + (0.75 * wanderForce);

							This.SetOrientation(newVelocity.GetAngle());
							This.SetVelocity(newVelocity);

							This.depositTimer -= 1;
							This.depositTimer = bound<int>(0, 10, This.depositTimer);
						}
					}
				}
			}
		}

		if (DeathProb(This.age, This.optimalAge, This.hungerLevel, nectarA) == true) {
			This.DeathReset();
		}
		
	}

	bool onlookerCheck(Vector2D loc)
	{
		bool return_dance = false;

		if (This.locationFound == false && This.onlooker == true && This.danced == false) {

			This.locationFound = true;
			This.danced = true;
			This.nectarLoc = loc;
			return_dance = true;

		}
		
		return return_dance;
	}

	virtual void OnCollision(WorldObject* obj)
	{
		HiveA* hive;
		Nectar* nectar;
		WorkerBeeA* worker;
		WorkerBeeB* enemy;

		if (IsKindOf(obj, hive)) {
			This.hungerLevel = 700;

			if (This.nectarCollected > 0) {
				hive->ForageReturn(This.nectarCollected);
				This.nectarCollected = 0;
				This.depositTimer = 7;
				This.soloDanceTimer = 25;
			}

			This.collided = true;
			This.col_timer = 3;
		}

		if (IsKindOf(obj, nectar)) {

			int nectar_return = nectar->Foraged();
			This.nectarLoc = nectar->GetLocation();

			if (nectar_return >= 60) {
				This.hungerLevel += 5;
				This.hungerLevel = bound<double>(0, 1000, This.hungerLevel);

				This.nectarCollected += nectar_return - 5;
				This.locationFound = true;		
			} else {
				This.nectarCollected += nectar_return;
				This.locationFound = false;
				This.danced = false;
			}
		}
		
		if (IsKindOf(obj, worker) && This.locationFound == true && This.onlooker == false) {
			bool new_danced = worker->onlookerCheck(This.nectarLoc);
			if (This.danced == false) {This.danced = new_danced;}
		}

		if (IsKindOf(obj, enemy)) {
			enemy->Attacked(This.role);
		}

		Animat::OnCollision(obj);
	}

protected:
	int role;
	bool onlooker;

	bool locationFound;
	Vector2D nectarLoc;
	int nectarCollected;
	bool danced;

	int depositTimer;
	int soloDanceTimer;
	
	double forager_age;
};

class BeeSimulation : public Simulation
{	
	// Colony A
	Group<WorkerBeeA> WorkA;
	Group<QueenBeeA> QueenA;
	Group<DroneBeeA> DroneA;

	// Colony B
	Group<WorkerBeeB> WorkB;
	Group<QueenBeeB> QueenB;
	Group<DroneBeeB> DroneB;

	Group<Nectar> FoodSupply;
	Group<HiveA> Hive_A;
	Group<HiveB> Hive_B;
	Group<MatingArea> MA;

public:
	BeeSimulation():
	WorkA(worker_popA),
	QueenA(1),
	DroneA(2),
	WorkB(worker_popB),
	QueenB(1),
	DroneB(2),
	FoodSupply(7),
	Hive_A(1),
	Hive_B(1),
	MA(1)
	{
		This.Add("Worker Bees Colony A", This.WorkA);
		This.Add("Queen Bees Colony A", This.QueenA);
		This.Add("Drone Bees Colony A", This.DroneA);

		This.Add("Worker Bees Colony B", This.WorkB);
		This.Add("Queen Bees Colony B", This.QueenB);
		This.Add("Drone Bees Colony B", This.DroneB);
		
		This.Add("Food Supply", This.FoodSupply);
		This.Add("Hive A", This.Hive_A);
		This.Add("Hive B", This.Hive_B);
		This.Add("Mating Area", This.MA);
		SetTimeSteps(5000);
	}
};

