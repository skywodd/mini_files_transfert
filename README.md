# Minimalist files transfert program
## by Skywodd

Written for a homework from my computer science school.
Not a real project, just a "code and forget" project for school.

Designed to be :
- linux only (use linux socket)
- multi clients (10 max by default)
- multi files (unlimited number of files per connection)
- error safe (handle most of common error case)
- single threaded (required by school subject)


/!\ DO NOT USE FOR PRODUCTION /!\
- NO SECURITY AT ALL
- NOT 100% TESTED (it's a school project ...)
- MADE FOR LEARNING PURPOSE

---

## Output example 
### Server side :
<pre>
# ./FileTransfert server 127.0.0.1 1234
[DEBUG] Listen on 127.0.0.1 : 1234
[DEBUG] Opening socket ...
[DEBUG] Binding socket ...
[DEBUG] Start listening ...
[DEBUG] Client accepted in slot 0
[DEBUG] Receiving header ...
[DEBUG] Filename : test.mp4
[DEBUG] File length : 13434880
[DEBUG] Client ready for data !
[DEBUG] End-of-file !
[DEBUG] Receiving header ...
[DEBUG] Filename : test2.mp4
[DEBUG] File length : 3606933
[DEBUG] Client ready for data !
[DEBUG] End-of-file !
[DEBUG] Receiving header ...
[DEBUG] Slot free !
</pre>

### Client side :
<pre>
# ./FileTransfert client 127.0.0.1 1234 ~/test.mp4 ~/test2.mp4
[DEBUG] Connection to 127.0.0.1 : 1234
[DEBUG] Opening socket ...
[DEBUG] Connection to target host ...
[DEBUG] Starting file process !
[DEBUG] Processing file : /home/skywodd/test.mp4
[DEBUG] Opening input file ...
[DEBUG] Relative filename : test.mp4
[DEBUG] File length : 13436946
[DEBUG] Sending header ...
[DEBUG] Sending data ...
[DEBUG] Processing file : /home/skywodd/test2.mp4
[DEBUG] Opening input file ...
[DEBUG] Relative filename : test2.mp4
[DEBUG] File length : 3606933
[DEBUG] Sending header ...
[DEBUG] Sending data ...
[DEBUG] Exiting ...
</pre>