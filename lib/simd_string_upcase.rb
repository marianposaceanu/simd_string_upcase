require 'simd_string_upcase/simd_string_upcase'

class String
  alias_method :original_upcase, :upcase

  def upcase
    SIMDStringUpcase.upcase(self)
  end
end
