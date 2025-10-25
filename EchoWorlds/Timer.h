#pragma once

class Timer
{
	float length, time;
	bool timedOut;
public:
	Timer(float length) : length(length), time(0.0f), timedOut(false)
	{

	}

	void step(float deltaTime)
	{
		time += deltaTime;
		if (time >= length)
		{
			time -= length;
			timedOut = true;
		}
	}

	bool isTimedOut() const
	{
		return timedOut;
	}

	float getTime() const
	{
		return time;
	}

	float getLength() const
	{
		return length;
	}

	void reset()
	{
		time = 0;
		timedOut = false;
	}
};