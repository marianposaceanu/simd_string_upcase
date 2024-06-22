require 'bundler/setup'
require 'rake/extensiontask'

Rake::ExtensionTask.new('simd_string_upcase') do |ext|
  ext.lib_dir = 'lib/simd_string_upcase'
end

task :default => :compile
