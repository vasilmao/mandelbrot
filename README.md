# mandelbrot
This program draws Mandelbrot set using SDL2.
compilation works on Linux
master       branch: branch with readme, +- eq to no_opt branch
no_opt       branch: no sse or avx optimization, compile with ./c_no_opt.sh   
opt          branch: sse and avx optimizations faster than no_opt,  compile with ./c_opt.sh   
opt_fast_cmp branch: sse and avx + fast double vector compare instead of masks, compile with ./c_opt.sh   
fps print includes only computing time (for no_opt also includes RenderDrawPoint)
