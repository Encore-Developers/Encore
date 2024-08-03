for i = 0,4,1
do
    OvershellTopLoc = Units:hpct(0.9)
    OvershellLeftLoc = leftside + (Units:winpct(0.2)*i)
    DrawRectangle(OvershellLeftLoc, OvershellTopLoc, Units:winpct(0.2), Units:hinpct(0.2), {255,255,255,2550})
end
