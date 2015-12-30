# Copyright (c) 2015, Sveta Smirnova. All rights reserved.
# 
# This file is part of filter_plugin.
#
# filter_plugin is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# filter_plugin is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with filter_plugin.  If not, see <http://www.gnu.org/licenses/>.

IF(DEFINED MYSQL_NO_DASH_VERSION)
    IF("5.7.4" VERSION_LESS ${MYSQL_NO_DASH_VERSION})
    MYSQL_ADD_PLUGIN(filter_plugin filter_plugin.cc
      MODULE_ONLY MODULE_OUTPUT_NAME "filter_plugin")
      SET_PROPERTY(TARGET filter_plugin PROPERTY CXX_STANDARD 11)
  ENDIF()
ENDIF()
