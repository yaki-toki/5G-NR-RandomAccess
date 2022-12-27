#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_USERS 10 // Number of users in the NOMA system

// Struct to represent a NOMA user
typedef struct {
  int id; // User ID
  int power; // Transmission power (dBm)
  double channelGain; // Channel gain (linear)
} NomaUser;

// Function to initialize a NOMA user
void initNomaUser(NomaUser *user, int id, int power, double channelGain) {
  user->id = id;
  user->power = power;
  user->channelGain = channelGain;
}

// Function to calculate the received signal strength for a NOMA user
double calcRxSigStr(NomaUser user, NomaUser transmitter) {
  // Calculate the received signal strength in dBm
  double rxSigStr = transmitter.power - 10 * log10(user.channelGain);
  return rxSigStr;
}

// Function to apply successive interference cancellation (SIC) to cancel interference from a NOMA user
void applySic(NomaUser *user, NomaUser interferer, double sicGain) {
  // Calculate the received signal strength of the interferer
  double rxSigStr = calcRxSigStr(*user, interferer);

  // Subtract the received signal strength of the interferer from the user's received signal strength
  user->power -= rxSigStr * sicGain;
}

int main() {
  // Initialize the NOMA users
  NomaUser users[NUM_USERS];
  for (int i = 0; i < NUM_USERS; i++) {
    // Assign random transmission powers and channel gains to each user
    int power = rand() % 20 + 10; // Random transmission power between 10 and 29 dBm
    double channelGain = (double) rand() / RAND_MAX; // Random channel gain between 0 and 1
    initNomaUser(&users[i], i+1, power, channelGain);
  }

  // Calculate the received signal strength for each user
  for (int i = 0; i < NUM_USERS; i++) {
    double rxSigStr = 0.0;
    for (int j = 0; j < NUM_USERS; j++) {
      if (i != j) {
        // Add the received signal strength from each other user to the total received signal strength
        rxSigStr += calcRxSigStr(users[i], users[j]);
      }
    }
    printf("Rx sig str for user %d: %f dBm\n", users[i].id, rxSigStr);
  }

  // Apply SIC to cancel the interference from each user for all other users
  for (int i = 0; i < NUM_USERS; i++) {
    for (int j = 0; j < NUM_USERS; j++) {
      if (i != j) {
        // Apply SIC to cancel the interference from user i for user j
        applySic(&users[j], users[i], 0.5); // Assume a SIC gain of 0.5
      }
    }
  }

  // Calculate the received signal strength for each user after applying SIC
  for (int i = 0; i < NUM_USERS; i++) {
    double rxSigStr = 0.0;
    for (int j = 0; j < NUM_USERS; j++) {
      if (i != j) {
        // Add the received signal strength from each other user to the total received signal strength
        rxSigStr += calcRxSigStr(users[i], users[j]);
      }
    }
    printf("Rx sig str for user %d after SIC: %f dBm\n", users[i].id, rxSigStr);
  }

  return 0;
}
