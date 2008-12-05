#!/usr/bin/env ruby

require "fileutils"
require "zip/zip"
require "zip/zipfilesystem"
require 'build_resources'

NAME = "urban_warfare"
PATH = "../../Releases"
number = 1

Dir.foreach(PATH) do |file|
    path = File.join(PATH, file)
    next unless File.file?(path)
    next unless file =~ /^#{NAME}\_([\d]+)\.zip$/
    old_number = Regexp.last_match(1).to_i
    number= old_number + 1 if old_number >= number
end

filename = File.join(PATH, sprintf("#{NAME}_%02d.zip", number))
FileUtils.rm_f( filename )

buildResources( "." )

Zip::ZipFile.open( filename, Zip::ZipFile::CREATE) do |zipfile|
    zipfile.add( "readme.txt", "readme.txt" )
    zipfile.add( "changes.txt", "changes.txt" )
    zipfile.add( "world.db3", "world.db3" )
    zipfile.add( "resources.dat", "resources.dat" )
    zipfile.add( "UrbanWarfare.exe",
                 File.join( "Release", "UrbanWarfare.exe" ) )
    zipfile.add( "hge.dll",
                 File.join( "..", "..", "ThirdParty", "hge", "hge.dll" ) )
    zipfile.add( "bass.dll",
                 File.join( "..", "..", "ThirdParty", "hge", "bass.dll" ) )
    zipfile.add( "sqlite3.dll",
                File.join( "..", "..", "ThirdParty", "sqlite", "sqlite3.dll" ) )
end
