#include <iostream>
#include "thinger/thinger.h"
#include <wiringPi.h>
#include <fstream>
#include <string>
using namespace std;

#define USER_ID             "********"
#define DEVICE_ID           "********"
#define DEVICE_CREDENTIAL   "********"

static const unsigned short signal = 18;
unsigned short data[5] = {0, 0, 0, 0, 0};
float celsius;
float humidity;
string warning;
string forcast;
ofstream myfile;

void writeHtmlFile(float celsius, float humidity)
{
    myfile.open ("/var/www/html/temperature.html");
    myfile << "<h1>Raspberry Pi based Climate Meter System - Web Server </h1>";
    myfile << "Temperature = \n" << celsius;
    myfile.close();

    myfile.open ("/var/www/html/humidity.html");
    myfile << "<h1>Raspberry Pi based Climate Meter System - Web Server </h1>";
    myfile << "Humidity = \n" << humidity;
    myfile.close();

}

short readData()
{
    unsigned short val = 0x00;
    unsigned short signal_length = 0;
    unsigned short val_counter = 0;
    unsigned short loop_counter = 0;

    while (1)
    {
        // Count only HIGH signal
        while (digitalRead(signal) == HIGH)
        {
            signal_length++;

            if (signal_length >= 200)
            {
                return -1;
            }

            delayMicroseconds(1);
        }


        if (signal_length > 0)
        {
            loop_counter++;



            if (signal_length < 10)
            {

                val <<= 1;		// 0 bit. Just shift left
            }

            else if (signal_length < 30)
            {

                val <<= 1;
            }

            else if (signal_length < 85)
            {

                val <<= 1;
                val |= 1;
            }

            else
            {

                return -1;
            }

            signal_length = 0;
            val_counter++;
        }


        if (loop_counter < 3)
        {
            val = 0x00;
            val_counter = 0;
        }


        if (val_counter >= 8)
        {

            data[(loop_counter / 8) - 1] = val;

            val = 0x00;
            val_counter = 0;
        }
    }
}



int main(int argc, char* argv[])
{

//    float humidity;
//    float celsius;
    float fahrenheit;
    short checksum;
    thinger_device thing(USER_ID, DEVICE_ID, DEVICE_CREDENTIAL);


    if (wiringPiSetupGpio() == -1)
    {
      return -1;
    }

    thing["Temperature"] >> [](pson& out){
        out = celsius;
    };
   
    thing["Humidity"] >> [](pson& out_hum){
       out_hum = humidity;
    };

    thing["Warning"] >> [](pson& out_warning){
       out_warning = warning;
    };

    thing["Forecast"] >> [](pson& out_forecast){
       out_forecast = forcast;
    };

    while(1)
    {
        thing.handle();
        pinMode(signal, OUTPUT);

        // Send out start signal
        digitalWrite(signal, LOW);
        delay(20);					// Stay LOW for 5~30 milliseconds
        pinMode(signal, INPUT);		// 'INPUT' equals 'HIGH' level. And signal read mode

        readData();		// Read DHT22 signal

        // The sum is maybe over 8 bit like this: '0001 0101 1010'.
        // Remove the '9 bit' data using AND operator.
        checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;

        // If Check-sum data is correct (NOT 0x00), display humidity and temperature
        if (data[4] == checksum && checksum != 0x00)
        {
            // * 256 is the same thing '<< 8' (shift).
            humidity = ((data[0] * 256) + data[1]) / 10.0;
            celsius = data[3] / 10.0;

            // If 'data[2]' data like 1000 0000, It means minus temperature
            if (data[2] == 0x80)
            {
                celsius *= -1;
            }

            fahrenheit = ((celsius * 9) / 5) + 32;

            // Display all data
            //cout << "TEMP: " << celsius << ", F: " << fahrenheit << "Humidity: " <<  humidity << endl;
            thing.stream(thing["Temperature"]);
            thing.stream(thing["Humidity"]);
            writeHtmlFile(celsius, humidity);

            if(celsius > 23 && celsius < 23.5)
            {
                thing.call_endpoint("email",thing["Temperature"]);
            }

            if(humidity > 70)
            {
                warning = "Storm Warning";
                forcast = "Cloudy";
                thing.stream(thing["Warning"]);
                thing.stream(thing["Forecast"]);

            } else
            {
                warning = "......";
                forcast = "......";
                thing.stream(thing["Warning"]);
                thing.stream(thing["Forecast"]);
            }

            
        }

        else
        {
            cout << "[x_x] Invalid Data. Try again." << endl;
        }

        delay(5000);
        //thing.stream(thing["Temperature"]);
    }

    return 0;
}

