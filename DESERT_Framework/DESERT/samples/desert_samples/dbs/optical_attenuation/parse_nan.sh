mkdir filter
for f in *; do cat "$f" | grep "[0-9]*.*,[0-9*].*,[0-9].*" > filter/$f; done

