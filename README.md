### Usage 
```
Usage: openmilight [hdlun:w:]

   -h                         Show this help
   -d                         Show debug output
   -l                         Listening (receiving) mode
   -u                         UDP mode (raw)
   -n NN<dec>                 Resends of the same message
   -w XXXXXXXXXXXXXXXXXX<hex> Complete message to send

 Inspired by sources from: - https://github.com/henryk/
                           - http://torsten-traenkner.de/wissen/smarthome/openmilight.php
                           - https://github.com/bakkerr/openmilight_pi

This code does no encryption/decryption, but it can send/receive packets with a length of 9 bytes.

For encryption/decryption I use the ruby code provided here:
https://github.com/sidoh/milight_decoding/blob/master/scripts/packet_transcoder.rb

More information about the encryption algorithm can be found here:
https://blog.christophermullins.com/2017/03/18/reverse-engineering-the-new-milightlimitlessled-2-4-ghz-protocol/

Quick and dirty test code in ruby:

require "./packet_transcoder.rb"
i=(((i||0)+1)%255)
str = PacketTranscoder.new.encode_packet([42, 32, 12, 34, 1,  4, i, 0]).map{|x| x.to_s(16).rjust(2, '0')}.join()
`./openmilight -d -w #{str}`


```
