#
# mesa.tcl
#

# show tcl information
#parray tcl_platform

# load config
mesa::guam setting GVWin
set displayWidth  [mesa::guam config displayWidth]
set displayHeight [mesa::guam config displayHeight]

package require Tk

wm withdraw .

toplevel .mesa

frame  .mesa.display -background lightblue -width $displayWidth -height $displayHeight -takefocus 1
frame  .mesa.panel
button .mesa.panel.run -text run
label  .mesa.panel.mp  -text 0000
label  .mesa.panel.display -text "screen $displayWidth x $displayHeight"

focus .mesa.display

pack .mesa.display -side top -anchor nw
pack .mesa.panel -side top -anchor w
pack .mesa.panel.display -side left
pack .mesa.panel.mp  -side left
pack .mesa.panel.run -side right

bind .mesa.display <KeyPress>      { keyPress %K }
bind .mesa.display <KeyRelease>    { keyRelease %K }
bind .mesa.display <ButtonPress>   { mouseButtonPress %b }
bind .mesa.display <ButtonRelease> { mouseButtonRelease %b }
bind .mesa.display <Motion>        { mouseMotion %x %y }

proc keyPress keySym {
    puts "keyPress $keySym"
}
proc keyRelease keySym {
    puts "keyRelease $keySym"
}
proc mouseButtonPress button {
    puts "mouseButtonPress $button"
}
proc mouseButtonRelease button {
    puts "mouseButtonRelease $button"
}
proc mouseMotion {x y} {
    puts "mouseMotion $x $y"
}