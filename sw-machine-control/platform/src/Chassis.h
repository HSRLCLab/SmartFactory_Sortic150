
#include <SharpIR.h>
#include <Adafruit_MotorShield.h>
#include <Component.h>
#include <Config.h>
#include <Debug.h>
#include <RunningMedian.h>

#ifndef Chassis_h
#define Chassis_h

class Chassis : public Component<int>
{
public:
  Chassis(Adafruit_DCMotor *motor, SharpIR *sensor) : sensor{sensor},
                                                      motor{motor}
  {
    componentData = 0;
  }

private:
  SharpIR *sensor;
  Adafruit_DCMotor *motor;
  int currentPosition{0};
  RunningMedian medianFilter{5};

protected:
  int loop()
  {
    medianFilter.add(sensor->getDistance());

    if (medianFilter.getCount() == 5)
    {
      currentPosition = medianFilter.getAverage();
      medianFilter.clear();
    }
    else
    {
      return currentPosition;
    }

    if (currentPosition > CHASIS_POS_MAX && targetValue > CHASIS_POS_MAX)
    {
      motor->run(RELEASE);
      motor->setSpeed(0);
      state = Invalid;
      debugLn("Max exceeded cannot go this way!" + String(currentPosition));

      return currentPosition;
    }
    if (currentPosition < CHASIS_POS_MIN && targetValue < CHASIS_POS_MIN)
    {
      motor->run(RELEASE);
      motor->setSpeed(0);
      state = Invalid;
      debugLn("Min exceeded cannot go this way!" + String(currentPosition));

      return currentPosition;
    }

    int distanceToTarget = abs(currentPosition - this->targetValue);

    motor->run(currentPosition > this->targetValue ? BACKWARD : FORWARD);
    motor->setSpeed(distanceToTarget > 5 ? (MAX_MOTORSPEED) : (distanceToTarget + MIN_MOTORSPEED));

    //Stop at target
    if (distanceToTarget <= CHASSIS_TOLERANCE)
    {
      debugLn("Position arrived!" + String(currentPosition));

      motor->run(RELEASE);
      motor->setSpeed(0);
      state = Finish;
    }

    return currentPosition;
  }
};

#endif