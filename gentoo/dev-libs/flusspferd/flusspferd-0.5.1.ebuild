# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: ebuild made by Philipp Reh, sefi@s-e-f-i.de$

RESTRICT="mirror"
DESCRIPTION="Flusspferd (German for Hippopotamus) is a C++ library providing
Javascript bindings."
HOMEPAGE="http://flusspferd.org/"

SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"
EAPI="2"
LICENSE="MIT"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="curl io sqlite tests xml"

RDEPEND="
	>=dev-lang/spidermonkey-1.7[unicode]
	>=dev-libs/boost-1.36
	curl? ( net-misc/curl )
	sqlite?  ( >=dev-db/sqlite-3 )
	xml? ( dev-libs/libxml2 )"
DEPEND="${RDEPEND}
	dev-lang/python"

src_configure() {
	local options=""

	use curl || options="${options} --disable-curl"
	use io || options="${options} --disable-io"
	use sqlite || options="${options} --disable-sqlite"
	use tests && options="${options} --enable-tests"
	use xml || options="${options} --disable-xml"

	./waf --with-cxxflags="${CXXFLAGS}" --prefix=/usr ${options} configure || die "waf configure failed"
}

src_compile() {
	./waf build "${MAKEOPTS}" -v || die "waf build failed"
}

src_install() {
	./waf --destdir=${D} install || die "waf install failed"
}