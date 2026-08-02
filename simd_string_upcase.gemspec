Gem::Specification.new do |s|
  s.name        = "simd_string_upcase"
  s.version     = "0.1.10"
  s.summary     = "SIMD-optimized string upcase for Ruby"
  s.description = "A SIMD-optimized replacement for Ruby's String#upcase method for x86_64 and Apple Silicon architectures"
  s.authors     = ["Marian Posaceanu"]
  s.email       = "contact@marianposaceanu.com"
  s.files       = Dir.glob("lib/**/*.rb") + Dir.glob("ext/**/*.{c,rb}")
  s.homepage    = "https://github.com/marianposaceanu/simd_string_upcase"
  s.license     = "MIT"
  s.extensions  = ["ext/simd_string_upcase/extconf.rb"]
  s.required_ruby_version = ">= 3.3.0"
  s.post_install_message = "SIMD String#upcase installed successfully!"

  s.requirements << "x86_64 or arm64 architecture"

  s.add_development_dependency "rake-compiler", "~> 1.3"
  s.add_development_dependency "benchmark-ips", "~> 2.15"
  s.add_development_dependency "minitest", "~> 5.25"
end
