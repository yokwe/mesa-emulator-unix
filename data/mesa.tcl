#
# mesa.tcl
#

# show tcl information
parray tcl_platform

# load config
mesa::guam setting GVWin
set displayWidth  [mesa::guam config displayWidth]
set displayHeight [mesa::guam config displayHeight]

package require Tk


frame .mesa
pack  .mesa -fill both -expand 1

# Create empty photo image to display
set display [image create photo -width $displayWidth -height $displayHeight]
# Allocate photo image block for display
# Save photo image name to update display image from emulator
mesa::guam display $display $displayWidth $displayHeight

label  .mesa.display -image $display -takefocus 1
pack   .mesa.display -side top

# To receive key event on .mesa.display
focus  .mesa.display

frame  .mesa.panel
pack   .mesa.panel -side top

button .mesa.panel.run -text run -takefocus 0
pack   .mesa.panel.run -side right

label  .mesa.panel.mp  -text 0000 -takefocus 0
pack   .mesa.panel.mp  -side left
label  .mesa.panel.display -text "screen $displayWidth x $displayHeight" -takefocus 0
pack   .mesa.panel.display  -side left


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
