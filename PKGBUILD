pkgname=artemis-git
pkgver=2d3a47f
pkgrel=1
pkgdesc="Native KDE AI coding client for Codex"
arch=('x86_64')
url="https://github.com/tylxr59/Artemis"
license=('MIT')
depends=('qt6-base' 'qt6-declarative' 'kirigami' 'syntax-highlighting'
         'knotifications' 'plasma-integration' 'git')
optdepends=('codex: OpenAI Codex CLI provider')
makedepends=('cmake' 'ninja' 'extra-cmake-modules')
provides=('artemis')
conflicts=('artemis')
source=("$pkgname::git+$url.git")
sha256sums=('SKIP')

pkgver() {
  cd "$pkgname"
  git describe --long --tags --always | sed 's/^v//;s/-/.r/;s/-/./'
}

build() {
  cmake -S "$pkgname" -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
  cmake --build build
}

check() {
  ctest --test-dir build --output-on-failure
}

package() {
  DESTDIR="$pkgdir" cmake --install build
  install -Dm644 "$pkgname/LICENSE" "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
