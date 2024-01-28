# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=8

if [[ ${PV} == 9999 ]]; then
	inherit git-r3
	EGIT_REPO_URI="https://github.com/beandog/bluray_info.git"
else
	SRC_URI="https://github.com/beandog/bluray_info/archive/${PV}.tar.gz -> ${P}.tar.gz"
	KEYWORDS="~amd64 ~arm ~arm64 ~hppa ~loong ~ppc ~ppc64 ~riscv ~x86 ~amd64-linux"
fi

DESCRIPTION="Blu-ray utilities: bluray_info, bluray_copy, bluray_player"
HOMEPAGE="https://github.com/beandog/bluray_info"

LICENSE="GPL-2"
SLOT="0"
IUSE="+libmpv"

DEPEND="media-libs/libbluray
	libmpv? ( media-video/mpv[libmpv,bluray] )"
RDEPEND="${DEPEND}"

src_configure() {
	econf \
		$(use_with libmpv)
}
