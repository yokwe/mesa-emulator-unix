#
# mesa.tcl
#

# show tcl information
parray tcl_platform

# load config
mesa::config GVWin
set config [mesa::config]
set displayWidth  [dict get $config displayWidth]
set displayHeight [dict get $config displayHeight]

package require Tk


frame .mesa
pack  .mesa -fill both -expand 1

# Create empty photo image to display
set display [image create photo -width $displayWidth -height $displayHeight]
# Allocate photo image block for display
# Save photo image name to update display image from emulator
mesa::display set $display

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


bind .mesa.display <KeyPress>      { mesa::event keyPress %N %K }
bind .mesa.display <KeyRelease>    { mesa::event keyRelease %N %K }
bind .mesa.display <ButtonPress>   { mesa::event buttonPress %b }
bind .mesa.display <ButtonRelease> { mesa::event buttonRelease %b }
bind .mesa.display <Motion>        { mesa::event motion %x %y }

# debug keysym
#bind . <KeyPress> { puts "KeyPress %N %K" }
