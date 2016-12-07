# Last.FM prepare
import os

import pandas as pd
from scipy.sparse import coo_matrix
from sklearn.model_selection import train_test_split

def clean_dataset(filename):
    """ so - i lied a little in the post about it being a one line operation
    to read in the dataset with pandas.
    it *should* be a one line operation, but there are a bunch of malformed
    lines in the dataset that trips up pandas. So lets read in the thing one
    line at a time, and strip out the bad data. After this runs it will be a
    one-liner to read in. honest this time """

    with open(filename + ".cleaned", "wb") as output:
        for i, line in enumerate(open(filename)):
            tokens = line.strip().split("\t")
            if len(tokens) != 4:
                print("wrong # of tokens", i)
                continue

            if not tokens[3].isdigit():
                print("non integer play count", i)
                continue

            if tokens[2] == '""':
                print("invalid artist id", tokens[2])
                continue

            # some lines contain carriage returns (without newlines), which
            # randomly messes pandas up
            line = line.replace('\r', '')

            output.write(line)

    os.rename(filename, filename + ".messy")
    os.rename(filename + ".cleaned", filename)


def read_data(filename):
    """ Reads in the last.fm dataset, and returns a tuple of a pandas dataframe
    and a sparse matrix of artist/user/playcount """
    # read in triples of user/artist/playcount from the input dataset
    data = pd.read_table(filename,
                         usecols=[0, 2, 3],
                         names=['user', 'artist', 'plays'])

    # map each artist and user to a unique numeric value
    data['user'] = data['user'].astype("category")
    data['artist'] = data['artist'].astype("category")

    # create a sparse matrix of all the users/plays
    plays = coo_matrix((data['plays'].astype(float),
                        (data['artist'].cat.codes.copy(),
                         data['user'].cat.codes.copy())))

    return data, plays


filename = 'usersha1-artmbid-artname-plays.tsv'

# remove broken lines
# clean_dataset(filename)

_, data = read_data(filename)

print("data: %s (%s)" % (data.shape, len(data.data)))

# a,b,c,d,e,f = train_test_split(
#     data.data, data.row, data.col, test_size=0.2, random_state=123)

# FIXME: may be should really split train and test?
data_train = data # coo_matrix((a, (c, e)))
data_test = data # coo_matrix((b, (d, f)))

print("train: %s (%s)" % (data_train.shape, len(data_train.data)))
print("mean: %s max: %s min: %s" % (
    data_train.data.mean(), data_train.data.min(), data_train.data.max()))

print("test: %s (%s)" % (data_test.shape, len(data_test.data)))

data_test.data.astype('float32').tofile('L.test.coo.data.bin')
data_test.col.tofile('L.test.coo.col.bin')
data_test.row.tofile('L.test.coo.row.bin')

data_train.row.tofile('L.train.coo.row.bin')

csr = data_train.tocsr()
csr.data.astype('float32').tofile('L.train.csr.data.bin')
csr.indices.tofile('L.train.csr.indices.bin')
csr.indptr.tofile('L.train.csr.indptr.bin')

csc = data_train.tocsc()
csc.data.astype('float32').tofile('L.train.csc.data.bin')
csc.indices.tofile('L.train.csc.indices.bin')
csc.indptr.tofile('L.train.csc.indptr.bin')
