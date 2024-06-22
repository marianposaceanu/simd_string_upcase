Gem::Specification.new do |s|
  s.name        = "simd_string_upcase"
  s.version     = "0.1.8"
  s.summary     = "SIMD-optimized string upcase for Ruby"
  s.description = "A SIMD-optimized replacement for Ruby's String#length method for x86_64 architectures"
  s.authors     = ["Marian Posaceanu"]
  s.email       = "contact@marianposaceanu.com"
  s.files       = Dir.glob("lib/**/*.rb") + Dir.glob("ext/**/*.{c,rb}")
  s.homepage    = "https://github.com/marianposaceanu/simd_string_upcase"
  s.license     = "MIT"
  s.extensions  = ["ext/simd_string_upcase/extconf.rb"]
  s.required_ruby_version = ">= 2.0.0"
  s.post_install_message = "SIMD String#upcase installed successfully!"

  s.requirements << "x86_64 architecture"

  s.add_development_dependency "rake-compiler", "~> 1.2"
  s.add_development_dependency "benchmark-ips"
end
