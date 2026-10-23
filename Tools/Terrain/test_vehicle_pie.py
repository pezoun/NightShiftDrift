"""Run in PIE with mouse released (Shift+F1). Injects Enhanced Input actions.
Writes measured samples; it does not claim a complete manual driving test.
"""
import unreal as u, math, json, os
W=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_game_world()
assert W, 'PIE must be running'
P=u.GameplayStatics.get_player_pawn(W,0)
M=P.get_component_by_class(u.ChaosWheeledVehicleMovementComponent)
mesh=P.get_component_by_class(u.SkeletalMeshComponent)
sub=[o for o in u.ObjectIterator() if isinstance(o,u.EnhancedInputLocalPlayerSubsystem) and not o.get_path_name().startswith('/Script/')][0]
actions={k:P.get_editor_property(n) for k,n in [('throttle','ThrottleAction'),('steer','SteeringAction'),('brake','BrakeAction'),('handbrake','HandbrakeAction')]}
root=u.Paths.convert_relative_path_to_full(u.Paths.project_dir())
data=json.load(open(root+'SourceArt/Terrain/region_manifest.json'))
pts=data['road_centerlines_m']['SouthLink']
heading=math.degrees(math.atan2(pts[3][1]-pts[0][1],pts[3][0]-pts[0][0]))
P.set_actor_location_and_rotation(u.Vector(pts[0][0]*100,pts[0][1]*100,pts[0][2]*100+140),u.Rotator(0,heading,0),False,True)
mesh.set_physics_linear_velocity(u.Vector())
mesh.set_physics_angular_velocity_in_degrees(u.Vector())
M.set_target_gear(1,True)
trans=P.get_component_by_class(u.DriftTransmissionComponent)
trans.set_editor_property('CurrentGear',1)
trans.set_brake_as_reverse_override(False)
start=u.GameplayStatics.get_time_seconds(W)
state={'samples':[],'last_sample':-1,'last_t':0,'injected':0,'finished':False}

def feed(throttle=0,steer=0,brake=0,handbrake=0):
    for key,value in [('throttle',throttle),('steer',steer),('brake',brake),('handbrake',handbrake)]:
        sub.inject_input_vector_for_action(actions[key],u.Vector(value,0,0),[],[])

def tick(dt):
    global TOUGE_TEST_HANDLE
    try:
        t=u.GameplayStatics.get_time_seconds(W)-start
        loc=P.get_actor_location();rot=P.get_actor_rotation();vel=P.get_velocity()
        speed=abs(M.get_forward_speed())*.036
        idx=min(range(len(pts)),key=lambda i:(loc.x/100-pts[i][0])**2+(loc.y/100-pts[i][1])**2)
        target=pts[min(len(pts)-1,idx+max(3,int(speed/3)))]
        angle=math.degrees(math.atan2(target[1]-loc.y/100,target[0]-loc.x/100))
        delta=(angle-rot.yaw+180)%360-180
        steering=max(-.65,min(.65,delta/48))
        if t<2: phase='settle';feed()
        elif t<8: phase='acceleration';feed(throttle=.7,steer=steering)
        elif t<13: phase='cornering';feed(throttle=max(.05,min(.55,(45-speed)*.04)),steer=steering)
        elif t<17: phase='braking';feed(brake=1 if M.get_forward_speed()>85 else 0,steer=steering*.5)
        elif t<23: phase='second_launch';feed(throttle=.7,steer=steering)
        elif t<24.2: phase='handbrake';feed(throttle=.25,steer=.5,handbrake=1)
        elif t<27: phase='recovery';feed(brake=1 if M.get_forward_speed()>85 else 0,steer=-.2)
        else:
            feed()
            state['finished']=True
            path=root+'Saved/Tests/touge_vehicle_samples.json';os.makedirs(os.path.dirname(path),exist_ok=True)
            with open(path,'w') as f:json.dump(state,f,indent=2)
            u.unregister_slate_post_tick_callback(TOUGE_TEST_HANDLE)
            print('TOUGE_VEHICLE_TEST_COMPLETE',path,'samples',len(state['samples']))
            return
        state['injected']+=1
        if t-state['last_sample']>=.1:
            slip=math.degrees(math.atan2(vel.y,vel.x))-rot.yaw if speed>3 else 0
            slip=(slip+180)%360-180
            state['samples'].append({'time_s':round(t,3),'phase':phase,'speed_kmh':round(speed,3),'signed_speed_kmh':round(M.get_forward_speed()*.036,3),'engine_rpm':round(M.get_engine_rotation_speed(),2),'gear':M.get_current_gear(),'roll_deg':round(rot.roll,2),'slip_deg':round(slip,2),'road_distance_m':round(math.hypot(loc.x/100-pts[idx][0],loc.y/100-pts[idx][1]),3),'location_cm':[loc.x,loc.y,loc.z],'steer':round(steering,3),'throttle_input':M.get_throttle_input(),'brake_input':M.get_brake_input(),'handbrake_input':M.get_handbrake_input()})
            state['last_sample']=t
    except Exception as exc:
        u.unregister_slate_post_tick_callback(TOUGE_TEST_HANDLE)
        print('TOUGE_VEHICLE_TEST_ERROR',str(exc))

TOUGE_TEST_HANDLE=u.register_slate_post_tick_callback(tick)
print('TOUGE_VEHICLE_TEST_STARTED')
