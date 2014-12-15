
Lightning Example Game
This example project is designed to show how to use various components provided by the Lightning Base.  It will be kept up to date with changes in the API, but in cases where there is a conflict with the documentation, the documentation is correct and this is wrong.  Refer to the documentation for the definitive word.

To run from the AppManager, going through the Base UI:
    # /Base/bin/AppManager
    - Click past all the UI screens.  (Make note of which player slot you selected if you are interested in verifying any of the data.)
    - At the game select scroll wheel, select LEG.
 
To run from the AppManager, skipping the Base UI:
    # /Base/bin/AppManager LEG/App.so n
        where n is 0, 1, or 2 for player slots, -1 for guest.
 
The Picker has five icons.  Use the arrow keys to navigate the icons, then press A to that activity.  Press A to return to the picker.  Press menu (or m on emulation) to return to the Base UI.
- Tutorials - a demonstration of playing tutorials.
    - press up and down to toggle the tutorials.
    - press left and right to switch from spelling to math tutorials.
    - press hint (or h on emulation) to play the tutorial. 
        - Currently the tutorials are logging events.  This should not be done by the apps.  It is handled by the tutorials themselves. 
- CYO - a demonstration of reading CYO information.
    - The screen display information about the CYO data available for this player.
    - To verify, check /home/lfu/nfsroot/Data/CYO/LEG/CYO/
        - It contains two files, one for profile 0, another for profile 1.
	- These files are being installed by the build scripts.  Eventually this will be installed by the PCApp or OmegaTerm.
- MDL - a demonstration of reading MDL directory information.
    - This screen displays micro-downloads available for this title  and for this player profile .
    - To verify, check /home/lfu/nfsroot/Data/MDL/LEG/
        - Each directory  that specifies the player in its meta.inf file under the ProfileAccess section  should be reflected on the display screen.
- Avatars - not functional yet.
- Bad Words - not functional yet.
- Logging - Logging is demonstrated in all the activities.
    - See "Lightning Logging TDD.doc" for definitive details.
 
 