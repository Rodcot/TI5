# Windows Network Traffic Monitoring and Control

Developers: Felipe Barbosa Hollerbach, Reynaldo Villar Garavini, Bruno Fonseca and Thiago Utsch Andrade

## Architecture

![image](https://github.com/Rodcot/TI5/blob/main/Arquitetura.png)

The monitoring computer is set to be "new" router where all the hosts will connect to via hotspot. With this, the local network traffic can be monitored and controlled.

To monitor the local traffic, the `packetTracker.ps1` script has to be run (steps on how to set it up below).

To control the local traffic, Windows Filtering Platform(WFP) filters were created to: limit bandwith, block applications internet access and to block specific IP addresses. All of the code for these filters can be easily changed to the desired application, bandwith or IP following the comments on the code. (steps on how to set it up below).

## Creating a Windows Hotspot Network 

Simply follow this [tutorial](https://www.howtogeek.com/214080/how-to-turn-your-windows-pc-into-a-wi-fi-hotspot/) to create a hotspot. It is very easy to setup on windows 7 and above due to the windows mobile hotspot option.

After setting up the hotspot on the monitoring computer all you need to do is access the internet through that hotspot on the user devices, the name and password of the network will be on the windows mobile hotspot screen.

## Monitoring | Capturing local traffic

By running `packetTracker.ps1` on the monitoring computer as a administrator, a `pcap` file is generated which can be further used for analysis using third-party Packet Analysis software like WireShark.

All you need to run the code locally is to run it as administrator and change these first lines of code to fit your computer.

```
# Change these variables according to your PC
$filenameETL = "NameOfTheWindowsLogFile.etl"
$filenameCAB = "NameOfTheWindowsEventsFile.cab"
$filenamePCAPNG = "NameOfThePCAP.pcapng"
$logFileWindows = "Directory\You\Want\The\PCAP\file\to\be\saved\"
$durationSeconds = 60 # 1 minute - Change to desired time for the script to run in seconds
```

## Control | Applying Traffic Control filters

To be able to build the code and run it as a driver you will need to install 4 different things:

[Visual Studio Code 2022](https://visualstudio.microsoft.com/vs/) - You need it to be able to build the WFP libraries easily. It is possible with other IDE's just much harder because of Microsoft. The community version is free so no worries about spending money.

[Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) - You will need it to be able to use the WFP libraries in Visual Studio Code. It is an easy installation following the tutorial on the link.

[Windows WDK](https://www.howtogeek.com/214080/how-to-turn-your-windows-pc-into-a-wi-fi-hotspot/) - The windows driver kit libraries will be necessary to run the WFP filters as a driver using the OSR Driver Loader. It is an easy installation following the tutorial on the link.

[OSR Driver Loader](https://www.osronline.com/article.cfm%5Earticle=157.htm) - A software to apply the created drivers into the Windows System. Very easy to use, just load the .sys files built and register or unregister them into the OS using the buttons.



### How to build and apply the filters

1) Create a project in Visual Studio Code 2022 for a Universal Windows Driver

2) Copy and paste the filter you want to build and apply into the projects folder

3) Change the code to fit your needs:

    3.1) For the App Blocking filter you will need to change line 64 to fit your situation:
    ```
    UCHAR targetApp = "Spotify.exe"; //Change this to the name of the Applications you want to block.
    ```
    
    3.2) For the Bandwith Throttling filter you will need to change line 63 to fit your situation:
    ```
    ULONG bandwithLimit = 10000000; //Change this to the bandwith limit you want in bytes. 10000000 bytes = 10 Mbps limit.
    ```
    3.3) For the Website Blocking filter you will need to change line 59 to fit your situation:
    ```
    ULONG targetIP = 0x2d3729df; //Change this to the IPs You want to block in hexadecimal.
    ```
    To easily see what a IP is in hex you can just use a converter like [this one](https://www.browserling.com/tools/ip-to-hex)
 
4) Build the project and go to the "NameOfYourProjectFOLDER"/x64/Debug and the .sys file will be there
5) Open the OSR Driver Loader load the .sys and register or unregister the filters you want to run or stop

## Credits

 - [OpenSource ETL2PCAPNG Project](https://github.com/microsoft/etl2pcapng/graphs/contributors) - This project made it so much easier to create a script for monitoring the network by using powershell commands that can only create ETL files, to convert them to the most used PCAP files.
 - [Programming LOL WFP series](https://www.youtube.com/watch?v=TNVpvcw_nyk) - Microsoft's documentation around WFP and in general are very incomplete and many of them with only one phrase telling you what to do and not how to do it. Programming LOL series did a great job of teaching the basics of WFP so we cound expand and create filters of our own.
