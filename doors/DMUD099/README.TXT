DOORMUD v0.99   [DOS/16bit]
=-=-=-=-=-=-=-=-=-=-=-=-=-=
Copyright 2003 Evan Elias.

This is the Sysop Documentation file for DoorMUD.  This document is also
available online in hypertext format at http://dmud.doormud.com/sysdocs.html ,
complete with configuration screenshots!  You may find that version of the
docs to be easier to use.


Upgrading from a previous version
---------------------------------
If you already have a previous version of DoorMUD installed on your BBS,
upgrading to the newest release is easy.  Simply unzip the new archive to your
old DoorMUD directory, overwriting older files as needed.  

If you installed the World Editor in v0.98, you must also download 
WORLD099.ZIP to upgrade your copy of the World Editor. 


==============================================================================


Introduction 
~~~~~~~~~~~~
DoorMUD: Land of the Forgotten is a text-based role-playing game of massive
proportions.  In an ancient, war-torn land, the demonic Warmaster of Chaos
lies locked away in a dungeon deep underground. His minions are forming armies
on the surface, preparing for the day when an evil champion will set him free.
Meanwhile, the holy forces of Order have finally forged a weapon capable of
slaying the Warmaster, but first they must find a hero powerful enough to 
wield the mighty blade. Thus, players are presented with a choice: Will they
strive to kill the Warmaster and bring Order to the land? Or shall they free
the Warmaster and doom the world to its destruction?

DoorMUD is an affordable, high-quality MUD (multi-user dungeon) that will run
on any BBS software capable of running doorgames. DoorMUD features fast-paced
real-time combat, excellent multinode interaction, a high-powered command
interface and a huge game world with over 2100+ rooms for players to explore.
It is actively supported, with new versions coming out on a regular basis.  A
World Editor add-on is also now included in the registered version, allowing
Sysops to modify the game world and create your own new dungeons, quests,
artifacts, and more!


==============================================================================


Contents
~~~~~~~~
1. Support features
2. Disclaimer
3. Setup
4. Configuration Options
5. Registration
6. Misc


==============================================================================


1. Support Features
~~~~~~~~~~~~~~~~~~~
- Can use built-in serial I/O, or can perform I/O through use of a fossil
  driver.  If a fossil driver is present, it will be detected automatically.
- Supports most common dropfile formats, including: chain.txt, dorinfo*.def,
  exitinfo.bbs, sfdoors.dat, callinfo.bbs, door.sys, and tribbs.sys.
- Smoothly supports use of telnet servers (COM/IP, Netmodem, etc), on any
  comm port up through COM99.
- Real-time multinode interaction -- players can easily communicate with each
  other on-line, and can join up in teams to defeat more difficult monsters.
- Lots of innovative sysop-friendly features to make your life easier, 
  including automatic daily backups and the ability to un-do game resets.
- Demo version allows training up through level 7, allowing users to get a 
  good feel for the game. Registration for the basic version costs only $20,
  and includes a World Editor program for modifying and/or expanding the game
  world.


==============================================================================


2. Disclaimer - Agreement
~~~~~~~~~~~~~~~~~~~~~~~~~
Users of this program must accept the following:

DOORMUD is supplied as is.  The author disclaims all warranties, expressed or
implied.  The author assumes no liability for damages, direct or 
consequential, which may result from the use of DOORMUD.

You may not modify or disassemble this software, in whole or in part.  Use 
of decompilers, hex editors, bytecode modifiers, and similar programs on any
DOORMUD executable file is strictly forbidden, and voids your license to run
DOORMUD.

==============================================================================


3. Setup
~~~~~~~~
In order to set up DoorMUD, perform the following steps:

Step 1:  Create a new directory just for DoorMUD, and copy all of the DoorMUD
         files to this directory.  (You may have already done this.)

Step 2:  Copy the file RUNMUD.BAT to wherever you usually put door batch
         files -- typically, your main BBS directory.

Step 3:  Edit RUNMUD.BAT so that the directories used are correct for your 
         system.  Remarks in the file will tell you which lines to fix.

Step 4:  Set up your BBS so it runs DoorMUD by calling RUNMUD.BAT.  You must
         pass the node number and location of the dropfile to the batch file,
         as follows:

            runmud.bat -n # -d XXX

         where # is the node number, and XXX is the location of the dropfile.  
         For example, if the caller is on node 2 and the dropfile is at 
         "c:\bbs\node2", you would want the BBS to call 
         "runmud.bat -n 2 -d c:\bbs\node2".

         Most BBS systems can automate the command line with special flags.  
         For example, a system running Renegade BBS should be set to use
         "runmud.bat -n %n -d %o".  Renegade automatically replaces "%n" with 
         the node number and "%o" with the dropfile location.  

         Some BBS systems have flags for the node number, but not dropfile 
         location.  This can work fine too.  For example, with the Virtual 
         Advanced BBS software, if your BBS directory is c:\va, use
         "runmud.bat -n %1 -d c:\va\temp\n%1" for your DoorMUD command line.


         Please consult the documentation for your BBS software to determine 
         what flags to use in order to properly pass the node number and 
         dropfile location.

Step 5:  To finish the installation, you must run the door in local mode by
         executing "dmud.exe -l".  The game will automatically finish its
         installation the first time you run it in local mode.  (Please note
         that for local play, use DMUD.EXE directly instead of RUNMUD.BAT)



==============================================================================


4. Configuration Options
~~~~~~~~~~~~~~~~~~~~~~~~
If you enter the door in local mode ("dmud.exe -l"), a special menu option
will become available that allows you to configure the game's settings.

At the top of the Configuration screen, you will see your Pre-registration
Code.  This is used for registration purposes if you wish to purchase the full
version of the game.

From the Configuration screen, you can change the following game settings:

[F] Monster Fights per Day:  
       Controls how many monster fights a player gets per day.  Every time a
       player kills a monster, one monster fight is used up.  Once a player
       runs out of monster fights, they can still continue to kill more 
       monsters if desired, but they won't gain any experience or gold for it.
       The default is 35 monster fights per day, which will entertain an
       experienced player for about 15 to 25 minutes per day.  

[M] Maximum Accumulated Fights:
       Unlike most other doorgames, unspent monster fights in DoorMUD add up
       from day to day.  This way, your users do not have to play every single
       day in order to maintain a high ranking.  This setting controls how 
       many unused fights a player can accumulate; the default is 100.  It is
       recommended that you set it to about 3 times as high as the daily fights
       setting.  But if you wish to just turn off fight accumulation entirely,
       set it to the same number as the daily fights setting.

[R] Reset the game
       Totally resets the entire game, deleting all players and returning all
       items to their original location.  Please note that there is no need to
       select this option when you first install the door.

[U] Undo previous reset
       Restores the game files prior to the previous reset. This option is 
       only available if the game has been reset before.


If you have purchased the DoorMUD Professional add-on, additional config
options are also available.  Please see the DoorMUD Professional documentation
for information on these settings.


==============================================================================


5. Registration
~~~~~~~~~~~~~~~
DoorMUD is shareware.  In the unregistered version, players may not advance 
past level 7, and can only explore about 700 of the 2100+ rooms in the game 
world.  If you register the game, your users can advance to any level
and can access the entire game world.  

Registration for the basic (4 line) version costs $20 US.  Registration now
includes a copy of the World Editor add-on program, which allows you to freely
modify and expand DoorMUD's game world!  Please see ADDONS.TXT for more
information.


HOW TO REGISTER ON-LINE:
Registering on-line is the fastest and most convenient method to purchase
DoorMUD.  Please see http://dmud.doormud.com/register.html for more information.


HOW TO REGISTER BY POSTAL MAIL:
1.  Print and fill out the registration form (REGISTER.TXT).  If you do not
    have a printer, you may handwrite the information.  Be sure to include
    your Preregistration Code.  This can be found by running CONFIG.EXE.
    Copy it EXACTLY as it appears.  
2.  Include a check or money order for $20 US.  If using a check, make it
    payable to "Evan Elias".
3.  E-mail RhythmNp@aol.com to inquire about the current registration mailing
    address.  Place your registration form and payment in an envelope and mail
    to this address.

If you pay with a personal check, please allow an extra week for the check to
clear.


==============================================================================


6. Misc
~~~~~~~

User license:
  This version of DoorMUD allows up to 4 simultaneous users/nodes to play the
  game at once.  This should be sufficient for most hobbyist boards.  But if
  you run a larger MUD-oriented BBS, you may be interested in the DoorMUD 
  Professional add-on, which allows for up to 16 simultaneous users.  Please
  see ADDONS.TXT for more information.

Monster Fights:
  There is intentionally no setting to allow for unlimited monster fights.
  However, you can set the Daily Monster Fights setting as high as 1000 if
  desired, which would allow for many hours of gameplay per day.  Setting it
  anywhere near this high is definitely NOT recommended though, unless you
  are using the DoorMUD Professional add-on to increase the experience point
  tables.  Otherwise, high Monster Fight settings will allow players to gain
  levels much too quickly.  In the basic version of DoorMUD, it is HIGHLY
  recommended that you leave the Monster Fights at the default setting of 35
  per day.

Automatic Backups:
  DoorMUD automatically makes backups of its vital data files on a regular
  basis, usually once per day.  If your data files ever become corrupted, 
  copy the three *.bak files (CREATURE.BAK, ITEM.BAK, CONFIG.BAK) over the
  *.dat files of the same name, to revert to the backups.  These backups are
  made just as a precaution; data file corruption is extremely rare.

USERINFO.TXT:
  This text file lists the real name of each user in the game, and shows how
  many days it has been since the user last played.  It is updated daily, when
  the game runs maintenance.  This is not a score file; it is intended to
  provide the Sysop with information as to which users are playing the game.

Command line options:  
 -D or -DROPFILE  - Door information file directory and/or filename.
 -N or -NODE      - Sets the node number to use.
 -L or -LOCAL     - Causes door to operate in local mode, without requiring a
                    door information (drop) file.  
 -TIMEOUT x       - Sets how long the game waits, in minutes, before kicking
                    out an inactive user.  Valid range is 4 min to 10 min; 
                    default is 5 minutes.
 -SILENT          - Operate in silent mode, with no local display.
 -NOSYSOP         - Disables the Sysop Configuration menu option in Local Mode.
                    Useful for *nix sysops who run all doors through redirected
                    local mode.
 -NOFOSSIL        - Disables use of FOSSIL driver, even if available.
 -MAXTIME x       - Sets the maximum number of minutes that any user will be
                    permitted to access the door, regardless of time left.
 -NOINTRO         - Skips the game's ANSI intro screen.
 -GRAPHICS        - Unless followed by 0 or N, turns on ANSI display mode.
                    Useful if, for whatever reason, the game is not detecting
                    users' ANSI settings properly.
 -HELP            - Lists several other available command lines options for use
                    when the dropfile isn't working properly or isn't present.



Support:
-=-=-=- 
Please consult one of the following sources if you are having difficulty setting
up the game or if you have any questions, comments, ideas, etc --

 - Visit the official DoorMUD webpage:  http://www.doormud.com
 - E-mail the game's programmer:        rhythmnp@aol.com

The web page contains many resources for players and sysops alike.  You may
want to give the Sysop FAQ a look if you are having difficulty with the game.



Thank You's:
-=-=-=-=-=-
Beta testing   -- Kevin Brown, Barry Bogart, Bryan Housel, Jeff Miller
ANSI graphics  -- Outworld Arts
16-bit Doorkit -- Brian Pirie
Web Hosting    -- awphosting.com


<End of file>