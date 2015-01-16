Mangler
========

# What Is Mangler?

Mangler is an open source VOIP client capable of connecting to Ventrilo 3.x servers. It is capable of performing almost all standard user functionality found in a Windows Ventrilo client. Mangler is maintained by Eric Connell. contributers include:

- Haxar (wishes to remain anonymous)
- Bob Shaffer II
- clearscreen/Daniel Sloof/danslo
- killy/Justin Pai

This project was forked by pi0 (pooya parsa) from version 1.2.5 to github


# How to get
as a quick start you can get apk builds from [here](http://pi0.ir/files/dl/mangler/apk/)


# Why?

Our project’s motto is: “No one should use our software… ever”

There are 100% open source alternatives to Ventrilo. We highly recommend Mumble for those that are looking into setting up this type of server. Mangler exists to bridge the gap for people who don’t have that choice. Ventrilo is the de facto standard for guild/clan conversation and Linux doesn’t have an implementation. The official Ventrilo client for Linux has been “in development” since 2005. Since it seems they have no intention of releasing a Linux client, we have taken up the task.

Many people ask if a server is the next step. Our answer is no. If you’re setting up a server, you should check out Mumble. The Ventrilo protocol includes a call home feature that checks the server’s license every time you connect. These tactics are anathema in open source ideology.

Reasons you should use Mumble:

- The server is free and does not have any of the bizarre limitations of Ventrilo servers
- It’s open source and is actively developed for all major platforms (Windows, Mac, Linux). iPhone and Android clients are currently in development
- It is truly low latency. Compared to Mumble, Ventrilo’s latency is astronomical
- The CELT codec is far superior to both speex and GSM
- Mumble does not have a forced call-home check.
- The Ventrilo network protocol itself is atrocious. It looks like it was written as a bad highschool programming project.
Other Contributors

Mangler is an extension of previous work at bringing Ventrilo to Linux. None of this would be possible without Luigi Auriemma’s open source code to encrypt and decrypt Ventrilo packets.

Also, Michael Sierks and Cris Favero put a lot of effort into deciphering a good chunk of the protocol.
