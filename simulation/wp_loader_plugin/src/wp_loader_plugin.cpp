#include "gazebo/physics/physics.hh"
#include "gazebo/common/common.hh"
#include "gazebo/gazebo.hh"
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/LocalCartesian.hpp>

namespace gazebo {
	class wpLoaderPlugin : public WorldPlugin {
		public: void Load(physics::WorldPtr _parent, sdf::ElementPtr _sdf) {

			double lat0 = 41.87986;
			double lon0 = -88.11111;
			
			
			std::stringstream in("1\n0\n\n0\n100\n41.88206119141624\n-88.1117585780525\n\n0\n100\n41.87741209971844\n-88.11345373414991\n\n0\n100\n41.87601411081567\n-88.10691987298964\n\n0\n100\n41.877987263416514\n-88.10180221818922\n\n0\n100\n41.882548447767\n-88.10417329095839\n\n0\n100\n41.877560117200424\n-88.1048752781636\n\n0\n100\n41.88353516265484\n-88.108008098293\n\n0\n50\n41.87835895242182\n-88.11004657714432\n");

			std::vector<std::vector<double>> waypoints;

			std::string line;
			std::string::size_type sz;
			std::string prev_line;
			
			bool lat = true;
			for(int i = 1; std::getline(in, line); i++) {
				if(i > 3) {
					if((i-2)%4 == 3 || (i-2)%4 == 0) {
						if(!lat) {
							waypoints[(waypoints.size()-1)][0] = std::stod(prev_line, &sz);
							waypoints[(waypoints.size()-1)][1] = std::stod(line, &sz);
							lat = true;
						}else {
							lat = false;
						}
						prev_line = line;
					}
					if((i-2)%4 == 2) {
						std::vector<double> item({0.0, 0.0, 0.0});
						waypoints.push_back(item);
						waypoints[(waypoints.size()-1)][2] = std::stod(line, &sz);
					}
				}
				if(line == "") i--;
			}

			const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();
			GeographicLib::LocalCartesian proj(lat0, lon0, 0, earth);


			sdf::SDF sphereSDF;
		    sphereSDF.SetFromString(
		       "<sdf version ='1.6'>\
		          <model name ='sphere'>\
		          	<static>1</static>\
		            <pose>0 0 0 0 0 0</pose>\
		            <link name ='link'>\
		              <pose>0 0 0 0 0 0</pose>\
		              <visual name ='visual'>\
		              	<transparency>0.2</transparency>\
		                <geometry>\
		                  <sphere><radius>20</radius></sphere>\
		                </geometry>\
		                <material>\
		                	<ambient>1 0 0 1</ambient>\
		                	<diffuse>1 0 0 1</diffuse>\
		                	<specular>1 0 0 1</specular>\
	                	</material>\
		              </visual>\
		            </link>\
		          </model>\
		        </sdf>");


			sdf::SDF lineSDF;
		    lineSDF.SetFromString(
		       "<sdf version ='1.6'>\
		          <model name ='cylinder'>\
		          	<static>1</static>\
		            <pose>0 0 0 0 0 0</pose>\
		            <link name ='link'>\
		              <pose>0 0 0 0 0 0</pose>\
		              <visual name ='visual'>\
		              	<transparency>0.2</transparency>\
		                <geometry>\
		                  <cylinder><radius>0.1</radius><length>5</length></cylinder>\
		                </geometry>\
		                <material>\
		                	<ambient>1 0 0 1</ambient>\
		                	<diffuse>1 0 0 1</diffuse>\
		                	<specular>1 0 0 1</specular>\
	                	</material>\
		              </visual>\
		            </link>\
		          </model>\
		        </sdf>");

			sdf::ElementPtr sphere = sphereSDF.Root()->GetElement("model");
			sdf::ElementPtr linec = lineSDF.Root()->GetElement("model");

			for(int i = 0; i < waypoints.size(); i++) {
				double x, y, z;
				proj.Forward(waypoints[i][0], waypoints[i][1], 0, x, y, z);

				double height_meters = waypoints[i][2]*0.3048;
				std::stringstream sspose;
				sspose << x << " " << y << " " << height_meters << " 0 0 0";
				std::string pose_string = sspose.str();

				sphere->GetAttribute("name")->SetFromString(std::to_string(i));
				sphere->GetElement("pose")->Set(pose_string);
				_parent->InsertModelSDF(sphereSDF);

				if(i != 0) {
					double x1, y1, z1;
					proj.Forward(waypoints[i-1][0], waypoints[i-1][1], 0, x1, y1, z1);
					z1 = waypoints[i-1][2]*0.3048;

					double x2 = x;
					double y2 = y;
					double z2 = height_meters;

					double xy = std::atan2((y2 - y1), (x2 - x1));

					double xy_distance = std::sqrt(std::pow((x2-x1), 2) + std::pow((y2-y1), 2) + std::pow((z2-z1), 2));

					double xy_z = std::atan2((z2 - z1), xy_distance);

					std::stringstream sslpos;
					sslpos << x1 << " " << y1 << " " << z1 << " " << xy_z-1.5708 << " " << 0 << " " << xy-1.5708;

					linec->GetAttribute("name")->SetFromString(std::to_string(i+waypoints.size()));
					linec->GetElement("pose")->Set(sslpos.str());
					linec->GetElement("link")->GetElement("visual")->GetElement("geometry")->GetElement("cylinder")->GetElement("length")->Set(xy_distance);
					std::stringstream sslp;
					sslp << "0 0 " << (xy_distance/2) << " 0 0 0";
					linec->GetElement("link")->GetElement("pose")->Set(sslp.str());
					_parent->InsertModelSDF(lineSDF);

				}
			}
			
		}
	};
	GZ_REGISTER_WORLD_PLUGIN(wpLoaderPlugin);
} 