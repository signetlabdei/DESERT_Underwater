set scr [info script]
proc initRovMobility { } {
	global ns woss_utilities position_rov opt defaultRNG

  set curr_y0 510
  set curr_x0 500

  set curr_y $curr_y0
  set curr_x $curr_x0

  set curr_lat0    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
  set curr_lon0    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  $curr_x ]
  set curr_depth  [expr -1.0 * $opt(depth_rov)]
  puts "$curr_x $curr_y $curr_depth"
  puts "$curr_lat0 $curr_lon0"
  $position_rov setLatitude_  $curr_lat0
  $position_rov setLongitude_ $curr_lon0
  $position_rov setAltitude_  $curr_depth

  set toa 0.0

  while { $toa < $opt(stoptime)} {
  	set curr_y [expr $curr_y0 + 90 * [$defaultRNG testdouble 1.0]]
  	set curr_x [expr $curr_x0 -100 + 200 * [$defaultRNG testdouble 1.0]]
  	set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
  	set curr_lon    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  $curr_x ]
  	set time_in_wp [expr $opt(rov_time_in_wp) + [$defaultRNG testdouble 1.0]*2*$opt(rov_time_in_wp)]
  	set toa      [$position_rov addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed_rov) $time_in_wp]
    set time_in_wp [expr 100 * $opt(rov_time_in_wp) + [$defaultRNG testdouble 1.0]*2*$opt(rov_time_in_wp)]
  	set toa      [$position_rov addWayPoint $curr_lat0 $curr_lon0 $curr_depth $opt(speed_rov) $time_in_wp]
  }
  

  # set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(time_in_wp)]
  # puts "waypoint 1  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(1) getX_]; y = [$position(1) getY_]; z = [$position(1) getZ_]; toa = $toa"

}

proc initLeaderMobility { } {
	global ns woss_utilities position_leader opt defaultRNG

	set curr_y0 500
  set curr_x0 500
  set curr_y $curr_y0
  set curr_x $curr_x0

  set curr_lat0    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
  set curr_lon    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  $curr_x ]
  set curr_depth  [expr -1.0 * $opt(depth_leader)]
  puts "$curr_x $curr_y $curr_depth"
  puts "$curr_lat0 $curr_lon"

  $position_leader setLatitude_  $curr_lat0
  $position_leader setLongitude_ $curr_lon
  $position_leader setAltitude_  $curr_depth
  set toa 0.0
  while { $toa < $opt(stoptime)} {
  	set curr_y [expr $curr_y0 - 50 * [$defaultRNG testdouble 1.0]]
  	set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
  	set time_in_wp [expr $opt(rov_time_in_wp) + [$defaultRNG testdouble 1.0]*2*$opt(rov_time_in_wp)]
  	set toa      [$position_leader addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed_rov) $time_in_wp]
  	set toa      [$position_leader addWayPoint $curr_lat0 $curr_lon $curr_depth $opt(speed_rov) $time_in_wp]
  	set curr_y [expr $curr_y0 + 30 * [$defaultRNG testdouble 1.0]]
  	set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
  	set toa      [$position_leader addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed_rov) $time_in_wp]
  	set toa      [$position_leader addWayPoint $curr_lat0 $curr_lon $curr_depth $opt(speed_rov) $time_in_wp]
  }

  # set toa      [$position_sink addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed) $opt(rov_time_in_wp)]
  # puts "waypoint 1  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = [$position(1) getX_]; y = [$position(1) getY_]; z = [$position(1) getZ_]; toa = $toa"

}

proc initDiverMobility { id } {
	global ns woss_utilities position_diver opt defaultRNG

	set max_movement_y 50
	set max_movement_x [expr $max_movement_y*2]

  set curr_y0 [expr 500 -50 * (1 + $id % $opt(n_diver)/2)]
  set curr_x0 [expr 500 -50 + 100 * ($id%2)]
  # set curr_y0 [expr 100 * $id]
  # set curr_x0 [expr 100 * $id]
  set curr_y $curr_y0
  set curr_x $curr_x0

  set curr_lat0    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
  set curr_lon0    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  $curr_x ]
  set curr_depth  [expr -1.0 * $opt(depth_divers)]
  puts "$curr_x $curr_y $curr_depth"
  puts "$curr_lat0 $curr_lon0"

  # $position_diver($id) setLatitude_  $curr_lat
  # $position_diver($id) setLongitude_ $curr_lon
  # $position_diver($id) setAltitude_  [expr -1.0 * $opt(depth)]

  $position_diver($id) addStartWayPoint $curr_lat0 $curr_lon0 $curr_depth $opt(speed_diver) 0.0

	set toa 0.0

  while { $toa < $opt(stoptime)} {
  	set curr_y [expr $curr_y0 + $max_movement_y * [$defaultRNG testdouble 1.0]]
  	set curr_x [expr $curr_x0 - $max_movement_x/2 + $max_movement_x * [$defaultRNG testdouble 1.0]]

  	set curr_lat    [ $woss_utilities getLatfromDistBearing  $opt(start_lat) $opt(start_long) 180.0 $curr_y ]
  	set curr_lon    [ $woss_utilities getLongfromDistBearing $opt(start_lat) $opt(start_long) 90.0  $curr_x ]
  	set time_in_wp [expr $opt(diver_time_in_wp) + [$defaultRNG testdouble 1.0]*2*$opt(diver_time_in_wp)]
  	set toa      [$position_diver($id) addWayPoint $curr_lat $curr_lon $curr_depth $opt(speed_diver) $time_in_wp]
  	set toa      [$position_diver($id) addWayPoint $curr_lat0 $curr_lon0 $curr_depth $opt(speed_diver) $time_in_wp]
  	#puts "$toa waypoint diver  lat = $curr_lat; long = $curr_lon ; depth = $curr_depth ; x = $curr_x; y = $curr_y;"
  	#puts "x,y,$curr_x,$curr_y"

  }

}