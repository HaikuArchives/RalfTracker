Name    : ralf_tracker_0.31a
Author  : Raphael MOLL and Hubert FIGUIERE

Version : 0.3.1 alpha1
Machine : BeBox with BeOS 1.1d7
Status  : freeware with sources publicly available.


Disclaimer (Raphael)
--------------------
I currently work in a small french company called MIPSYS.
There is no link between this software and MIPSYS.
I originally started the port of the amiga tracker as a play. Beside
that, it gave me some interesting experience about the BeBox
programming, especially for threads and BAudioSubscribers.

Disclaimer (Hubert)
-------------------
I helped Raphael by fixing bugs and releasing this version. I'll also
improve several things in the tracker's interface, since Ralf do not 
have lot of time to give to Ralf Tracker.

Important
---------
This software is a preliminary version. Anything can happen. It can
crash, but it can also hangs your computer at any moment. I am not
responsible of any damage that you, your work, your computer or anything
else on Earth can be subjected to due to the usage or misuage of this
software.

Subject
-------
ralf_tracker is a so called "sound tracker", which plays music files
called "modules" and output them to the audio output of your prefered
BeBox.

Distribution
------------
There are two possible archives.

The first one, "ralf_tracker_0.31a_app.tgz" contains the application.
As an example, a module called "cripto.mod" is bundled with the archive.
More modules can be found on every ftp site on the Internet. Just ask
for ".mod" with Archie. Modules typically can be found in music-related
subdirectories or in Aminet mirrors.

The second archive will be called "ralf_tracker_0.31a_src.tgz" and will
contain the whole sources, with the BeIDE DR1 projects. If you want to 
compile it using CW for Mac, just look atr the Be version of the project
files.

History
-------
The ralf_tracker is heavily based on a C-source tracker running under
several unixes (Linux, Solaris, HP, etc.), created by Marc.Espie@ens.fr.
See file tracker-3_19.readme (yes, I do not already use the version 4!)

How to use
----------
This time it is very easy to use it since I coded the inferface...
Just launch the application and drop files from the browser onto the
list (on the left).
Then select a file in the list and 1) double-clic it, or 2) click the
"play" button. This make the tracker starting playing that file.
The list is a play list. This means that when a module is finished, the
tracker plays the next one in the list or wraps.
I added the items in the main menu, and so I provide some shortcuts.

Bugs
----
*Still to be coded are the four buttons to manage the list (load, save,
reorder).
*Sometimes the next button doesn't work : when you press it, instead of
playing the next module, it stops. Workaround : just click stop and then
click play again.

*There was an old bug that made the application crash if the quit button
was used while playing a module. I think I exterminated that bug but I'm
not quite sure. In fact it crashes sometimes when yopu quit [Hub]

*When you load several files, the window is locked while it processes the 
entry names.

The End
-------
Just play with that and tell us.


o-------------------------------------------------------------o
Raphael MOLL
------------
  e-mail for personal information : moll@linux.utc.fr
web page for personal information : http://linux.utc.fr/~moll/
o-------------------------------------------------------------o


Hubert FIGUIERE
	Hubert.Figuiere@inforoute.cgs.fr
	figuiere@kagi.com
	http://www.inforoute.cgs.fr/figuiere