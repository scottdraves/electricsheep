# Electric Sheep Frequently Asked Questions #

### Top Questions ###
  * [Why don't I see any sheep, or just one or two sheep?](FAQ#Getting_Sheep.md)
  * [Does it crash on your old PC?](FAQ#Problems_with_Old_Laptops_and_PCs.md)
  * [I don't see my name on the credit page, am I rendering?](FAQ#Credit_Vanishes_or_Missing.md)
  * [Why don't I render?](FAQ#Not_Rendering.md)
  * [Do the Sheep put my computer at risk to viruses?](FAQ#Security_and_Viruses.md)
  * [Why not use BitTorrent?](FAQ#BitTorrent.md)
  * [What Are Supported Command Line Arguments?](FAQ#Command_Flags.md)
<p><br>
<hr />
<p><br>
<h2>Getting Sheep</h2></li></ul>

Are you not getting enough sheep? If you just installed the software your cache directory is empty, the client needs to download its first sheep. Leave the screen saver running overnight (make sure to disable power settings that might hibernate your computer).<br>
<br>
The server is heavily loaded, it might take some time before it downloads many sheep. Please be patient.<br>
<br>
If you have a laptop make sure the power saving settings are not turning off the screen-saver.<br>
<br>
<h3>Manual Download</h3>

If you don't want to wait, you can download and install some sheep manually. There are two ways to do this:<br>
<br>
1. To download a packs of 100s, follow the directions in the <a href='http://community.electricsheep.org/node/247'>forum</a>.<br>
<br>
2. To download one at a time, go to the <a href='http://v2d7c.sheepserver.net/cgi/status.cgi'>server</a> , pick a sheep, click on the icon to go to its own page. Then click again (control-click on mac, or right-click on windows) on the full-size image to save it to your hard disk in the same location as you put the packs (see the link above).<br>
<p><br>
<hr />
<p><br>
<h2>Problems with Old Laptops and PCs</h2>

If on Windows, Electric Sheep crashes instantly when run, try turning on "Direct Draw" in the advanced settings tab. If that doesn't work, turn on logging then check the log file (in Windows it is in C:\Documents and Settings\All Users\Application Data\ElectricSheep\Logs). If you get 'Failed to create Direct3D 9 device', your graphics chipset probably can't run it.<br>
<p><br>
<hr />
<p><br>
<h2>Credit Vanishes or Missing</h2>

The <a href='http://v2d7c.sheepserver.net/cgi/status.cgi?&detail=credit'>credits page</a> only tallies the contributions to the current flock. Your credit dies with the sheep. If it's downloading your client is probably rendering fine.<br>
<br>
There's also a bug in how the server tallies renders where only 1 in 4 frames is counted. That's a lot, but it is fair, so it should not affect the ranking.<br>
<br>
<p><br>
<h2>Not Rendering</h2>

Most of the time, most users do not render.  This is fine, don't worry about it.  We have so many users that there is not enough work for everyone at all times, so most computers are idle.  This is good, as it means it uses less power and is greener!    In order to make it onto the leader-board you need persistence and luck.<br>
<p>
Sometimes when we are doing a high-definition render, then the network will be saturated, and everyone renders.  When this happens is unpredictable, as is the rendering itself.<br>
<p><br>
<hr />
<p><br>
<h2>Security and Viruses</h2>

<h3>Do the Sheep put my computer at risk to viruses?</h3>

Nobody's had any problems, and the Linux version was audited by a security company, so I think it's safe.  The Electric Sheep is listed on <a href='http://www.nonags.com/'>nonags.com</a>, which screens its content for spam and virus risks.  However, I cannot guarantee that a criminal hasn't broken into my web site and infected the installer. Anytime you download and run software from the internet there's a risk.<br>
<br>
Some people have reported that their firewall alerted on the Sheep, but that was an old version many years ago that had P2P networking.  We think that confused some security software.<br>
<br>
<h3>Does it share or use any information from my computer?</h3>

No it does not violate your privacy in any way.  The only data sent to the server are your votes (if you choose to). And the rendered frames, which are made according to instructions received on the server, and are the same no matter who would have done them.<br>
<br>
<h3>What does it do over the network, and what servers does it contact?</h3>

The screensaver client does access a number of servers over the internet, in order to coordinate the work of creating the sheep, as well as downloading them.  It uses the HTTP protocol over port 80, which is quite generic and standard.  You can set a proxy in the settings dialog as needed.  Since it only makes outgoing connections, you shouldn't need to configure any firewall you might have.  But if you are curious, or you use something like the Little Snitch to monitor and control your network access, the domain names and IP#s used are listed here:<br>
<br>
<table><thead><th> number </th><th> name </th></thead><tbody>
<tr><td> 74.207.242.239 </td><td> <code>*</code>.sheepserver.net </td></tr>
<tr><td> 207.241.<code>*</code>.<code>*</code> </td><td> <code>*</code>.us.archive.org </td></tr></tbody></table>


<p><br>
<hr />
<p><br>
<h2>BitTorrent</h2>

<h3>Why not use BitTorrent to download the sheep?</h3>

We tried that already and only 10-15% of our users were able to seed, which wasn't enough to make it work. Instead we are focusing on the HTTP which is much more reliable, and adding a subscription system to pay for our bandwidth. Our goal is to keep it free for everyone, and at the same time make it sustainable ie pay our own bills. This is the <a href='http://en.wikipedia.org/wiki/Freemium'>freemium</a> model.<br>
<br>
See <a href='http://community.electricsheep.org/node/787'>http://community.electricsheep.org/node/787</a>


<p><br>
<hr />
<p><br>
<h2>Command Flags</h2>

<h3>What Are Supported Command Line Arguments?</h3>

On Windows, the arguments to the electric sheep executable (es.exe in C:\Windows\) are as follows:<br>
<ul>
  <li>None - Starts the settings window</li>
  <li>'R' - Start in fullscreen</li>
  <li>'T' - Start windowed</li>
  <li>'X' - Same as above, but allows multiple instances</li>
  <li>'S' - Starts in screensafer mode</li>
 </ul>
