export GAZEBO_PLUGIN_PATH=$HOME/.gazebo/models/gps_plane/manual_control_plugin/build/:${GAZEBO_PLUGIN_PATH}
export GAZEBO_PLUGIN_PATH=$HOME/.gazebo/models/gps_plane/autopilot_plugin/build/:${GAZEBO_PLUGIN_PATH}
export GAZEBO_PLUGIN_PATH=$HOME/files/projects/GPS-plane-unit/simulation/wp_loader_plugin/build/:${GAZEBO_PLUGIN_PATH}
export GAZEBO_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/gazebo-9/plugins:${GAZEBO_PLUGIN_PATH}
gazebo --verbose north_side.world
