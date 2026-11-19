"""rooms.py - bibliotheque de salles 20x15 tuiles, assemblees au hasard par le jeu (niveaux semi-aleatoires).
Legende : # solide  = plateforme traversable  ^ pique  e ennemi(aleatoire) E ennemi sur  c coffre  f fiole  h coeur
          o pieces  T torche  S depart  D porte de sortie  d porte cachee (arene)  B boss  . vide
Regle de conception : le sol (rangees 13-14) relie toujours les deux bords ; fosses <= 3 tuiles, piques <= 3 tuiles."""
W, H, FLOOR = 20, 15, 13

class Room:
    def __init__(s, name):
        s.name = name
        s.g = [['.'] * W for _ in range(H)]
        for x in range(W):
            for y in (0, 1, 13, 14):
                s.g[y][x] = '#'
    def p(s, row, c0, c1):
        for x in range(c0, c1 + 1): s.g[row][x] = '='
        return s
    def pit(s, c0, c1):
        assert c1 - c0 + 1 <= 3, "fosse trop large"
        for x in range(c0, c1 + 1):
            s.g[13][x] = '.'; s.g[14][x] = '.'
        return s
    def spk(s, c0, c1):
        assert c1 - c0 + 1 <= 3, "piques trop larges"
        for x in range(c0, c1 + 1): s.g[12][x] = '^'
        return s
    def blk(s, c0, c1, h):
        for x in range(c0, c1 + 1):
            for y in range(FLOOR - h, FLOOR): s.g[y][x] = '#'
        return s
    def at(s, ch, row, col):
        s.g[row][col] = ch
        return s
    def e(s, r, c): return s.at('e', r, c)
    def E(s, r, c): return s.at('E', r, c)
    def c(s, r, c): return s.at('c', r, c)
    def f(s, r, c): return s.at('f', r, c)
    def h(s, r, c): return s.at('h', r, c)
    def o(s, r, c): return s.at('o', r, c)
    def T(s, r, c): return s.at('T', r, c)
    def rows(s): return [''.join(r) for r in s.g]

def build():
    R = []
    R.append(Room('depart').at('S', 12, 2).T(5, 3).T(5, 9).T(5, 16).c(12, 15))
    R.append(Room('sortie').at('D', 12, 16).T(5, 4).T(5, 13).T(5, 18).e(12, 6).e(12, 10).e(12, 13))
    R.append(Room('arene').p(9, 2, 5).p(9, 14, 17).T(4, 4).T(4, 15).at('B', 12, 13).at('d', 12, 18))
    # ---- salles du milieu
    R.append(Room('couloir').T(5, 2).T(5, 9).T(5, 16).e(12, 3).e(12, 9).e(12, 15).o(12, 6))
    R.append(Room('escalier').p(11, 1, 4).p(11, 15, 18).p(9, 5, 8).p(9, 11, 14).p(7, 8, 11)
             .c(6, 9).e(12, 7).e(12, 12).e(10, 3).T(4, 2).T(4, 17))
    R.append(Room('fosse').pit(8, 10).p(11, 8, 10).e(12, 4).e(12, 14).e(12, 16).T(5, 3).T(5, 16).h(12, 12))
    R.append(Room('piques').spk(6, 8).p(11, 5, 9).e(12, 3).e(12, 13).e(12, 16).h(12, 18).T(5, 4).T(5, 15))
    R.append(Room('tours').p(11, 2, 5).p(9, 6, 9).p(7, 10, 13).p(5, 14, 17).c(4, 16)
             .e(12, 8).e(12, 17).e(10, 3).e(8, 7).e(6, 11).T(6, 1).T(9, 18))
    R.append(Room('colonnes').blk(6, 7, 2).blk(12, 13, 2).e(12, 4).e(12, 10).e(12, 16)
             .T(5, 2).T(5, 10).T(5, 17).o(12, 9))
    R.append(Room('double fosse').pit(5, 6).pit(13, 14).p(11, 5, 6).p(11, 13, 14)
             .e(12, 3).e(12, 9).e(12, 10).e(12, 17).T(5, 4).T(5, 15))
    R.append(Room('autel').blk(8, 11, 2).c(10, 9).T(8, 7).T(8, 12).e(12, 3).e(12, 5).e(12, 15).e(12, 17).f(12, 14))
    R.append(Room('volantes').p(11, 3, 5).p(11, 9, 11).p(11, 15, 17).p(9, 6, 8).p(9, 12, 14)
             .e(8, 7).e(8, 13).e(10, 10).e(12, 2).e(12, 18).o(10, 4).o(10, 16).T(5, 3).T(5, 16))
    R.append(Room('piege').spk(4, 5).spk(14, 15).pit(9, 10).p(11, 8, 11).e(12, 7).e(12, 12).c(12, 17).T(5, 2).T(5, 12))
    R.append(Room('passage').at('S', 12, 2).at('N', 12, 6).at('M', 12, 11).at('D', 12, 17).T(5, 3).T(5, 9).T(5, 15))
    return R

def validate(rooms):
    for r in rooms:
        rows = r.rows()
        assert len(rows) == H and all(len(x) == W for x in rows), r.name
        for y in range(2, 13):            # les bords doivent rester ouverts
            assert rows[y][0] != '#' and rows[y][W - 1] != '#', (r.name, y)
        assert rows[13][0] == '#' and rows[13][W - 1] == '#', r.name
        for y, row in enumerate(rows):
            for x, ch in enumerate(row):
                if ch in 'eEcfhSBNM' and y < 14:     # il faut un appui sous la case
                    below = rows[y + 1][x]
                    assert below in '#=', (r.name, 'pas d appui sous', ch, y, x, below)
    return True

if __name__ == '__main__':
    rs = build(); validate(rs)
    for r in rs:
        print('---', r.name); print('\n'.join(r.rows()))
