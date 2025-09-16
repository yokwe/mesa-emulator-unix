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

proc create_photo_image_data {w h} {
    set rows {}
    for {set y 0} {$y < $h} {incr y} {
        set row {}
        for {set x 0} {$x < $w} {incr x} {
            lappend row {#fff}
        }
        lappend rows $row
    }
    return $rows
}

frame .mesa
pack  .mesa -fill both -expand 1

set display [image create photo -width $displayWidth -height $displayHeight]
$display put [create_photo_image_data $displayWidth $displayHeight]
label .mesa.display -image $display -takefocus 1
pack  .mesa.display -side top

focus .mesa.display

frame .mesa.panel
pack  .mesa.panel -side top

button .mesa.panel.run -text run -takefocus 0
pack .mesa.panel.run -side right

label  .mesa.panel.mp  -text 0000 -takefocus 0
pack .mesa.panel.mp  -side left
label  .mesa.panel.display -text "screen $displayWidth x $displayHeight" -takefocus 0
pack .mesa.panel.display  -side left


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
