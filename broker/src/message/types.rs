use std::cmp::PartialEq;

use crate::message::result::{Error, Result};

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum Type {
    Produce,
    Consume,
}

impl Type {
    pub fn decode(enc: u16) -> Result<Type> {
        if enc == Type::Produce as u16 {
            Ok(Type::Produce)
        } else if enc == Type::Consume as u16 {
            Ok(Type::Consume)
        } else {
            Err(Error::DecodeError("failed to decode type"))
        }
    }

    pub fn encode(&self) -> u16 {
        *self as u16
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn encode() {
        assert_eq!(Type::Produce.encode(), 0);
        assert_eq!(Type::Consume.encode(), 1);
    }

    #[test]
    fn decode_ok() {
        assert_eq!(Type::decode(0).unwrap(), Type::Produce);
        assert_eq!(Type::decode(1).unwrap(), Type::Consume);
    }

    #[test]
    #[should_panic]
    fn decode_error() {
        Type::decode(7).unwrap();
    }
}
