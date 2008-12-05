#!/usr/bin/env ruby

require "fileutils"
require "zip/zip"
require "zip/zipfilesystem"

def buildResources( target )
    resources_file = File.join( target, "resources.dat" )
    FileUtils.rm_f( resources_file )
    Zip::ZipFile.open( resources_file, Zip::ZipFile::CREATE) do |zipfile|
        Dir.entries( "Resources" ).each do |filename|
            next unless filename =~ /\.(png|mod|res|fnt|psi|wav|mp3)$/
            zipfile.add( filename, File.join( "Resources", filename ) )
        end
    end
    world_file = File.join( target, "world.db3" )
    FileUtils.rm_f( world_file )
    FileUtils.cp( "Resources/world.db3", world_file )
end

if __FILE__ == $0
    target = "."
    target = ARGV[0] if ARGV.length > 0
    buildResources( target )
end
