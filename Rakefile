require 'bundler/setup'
require 'rake/extensiontask'
require 'rake/testtask'

Rake::ExtensionTask.new('simd_string_upcase') do |ext|
  ext.lib_dir = 'lib/simd_string_upcase'
end

Rake::TestTask.new(:test) do |test|
  test.libs << 'test'
  test.pattern = 'test/**/*_test.rb'
end

task default: :compile
task test: :compile
